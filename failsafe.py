# -*- coding: utf-8 -*-
import rospy
from mavros_msgs.msg import State, RCIn
from mavros_msgs.srv import SetMode
from clover import srv
from std_srvs.srv import Trigger
import threading
from collections import deque
from math import isnan, sqrt

rospy.init_node('failsafe_node')

_get_telemetry = rospy.ServiceProxy('get_telemetry', srv.GetTelemetry)
_navigate = rospy.ServiceProxy('navigate', srv.Navigate)
_land = rospy.ServiceProxy('land', Trigger)
_set_mode = rospy.ServiceProxy('/mavros/set_mode', SetMode)

# Global variables
data_lock = threading.Lock()
get_telemetry_lock = threading.Lock()
is_armed = False
failsafe_active = False
telemetry_deque = None
in_failsafe_sequence = False
telemetry_timer = None
current_mode = None

def get_telemetry():
    with get_telemetry_lock:
        tel = _get_telemetry(frame_id = "aruco_map")
    return tel

# Navigate to point with wait and timeout
def navigate(coord):
    if isnan(coord.x) or isnan(coord.y) or isnan(coord.z):
        rospy.loginfo("Drop navigation point because of NaN")
        return
    rospy.loginfo(f'Navigating to x={coord.x} y={coord.y} z={coord.z}')
    timeout = 2
    delta = 0.3 
    start_time = rospy.get_time()
    _navigate(x=coord.x, y=coord.y, z=coord.z, yaw=float("nan"), speed=1, frame_id="aruco_map", auto_arm=True)
    while not rospy.is_shutdown():
        with get_telemetry_lock:
            telem = _get_telemetry(frame_id='navigate_target')
        if sqrt(telem.x ** 2 + telem.y ** 2 + telem.z ** 2) < delta:
            rospy.loginfo("Point reached")
            break
        if rospy.get_time() - start_time > timeout:
            rospy.loginfo("Timeout, point not reached")
            break
        rospy.sleep(0.2)

def land():
    rospy.loginfo('Initiating landing sequence.')
    _land()

def give_control():
    rospy.loginfo('Giving control back to operator.')
    if not _set_mode(custom_mode = current_mode):
        rospy.logfatal("Fail to return control to operator! Landing")
        land()

def telemetry_collection(event):
    global telemetry_deque
    with data_lock:
        if is_armed and not failsafe_active and telemetry_deque is not None:
            telemetry = get_telemetry()
            telemetry_deque.append(telemetry)
            rospy.loginfo('Telemetry added to deque.')

def start_telemetry_collection():
    global telemetry_timer
    if telemetry_timer is None:
        telemetry_timer = rospy.Timer(rospy.Duration(1), telemetry_collection)  # 1Hz

def stop_telemetry_collection():
    global telemetry_timer
    if telemetry_timer is not None:
        telemetry_timer.shutdown()
        telemetry_timer = None

def failsafe_sequence():
    global in_failsafe_sequence, telemetry_deque
    with data_lock:
        if in_failsafe_sequence:
            return  # Failsafe sequence is already running
        in_failsafe_sequence = True
        rospy.loginfo('Starting failsafe sequence.')
    while True:
        with data_lock:
            if not failsafe_active:
                # Scenario 1: Failsafe deactivated
                rospy.loginfo('Failsafe deactivated during sequence.')
                give_control()
                break
            if telemetry_deque is None or len(telemetry_deque) == 0:
                # Scenario 2: Deque is empty
                rospy.loginfo('Telemetry deque is empty.')
                land()
                break
            coordinate = telemetry_deque.pop()
        # Navigate to the coordinate outside the lock
        navigate(coordinate)
    with data_lock:
        in_failsafe_sequence = False
        rospy.loginfo('Failsafe sequence completed.')

def state_callback(msg):
    global is_armed, telemetry_deque, current_mode
    with data_lock:
        if msg.armed and not is_armed:
            # Drone armed
            current_mode = msg.mode
            rospy.loginfo(f'Drone armed. Flight mode is {current_mode}')
            is_armed = True
            telemetry_deque = deque()
            start_telemetry_collection()
        elif not msg.armed and is_armed:
            # Drone disarmed
            rospy.loginfo('Drone disarmed.')
            is_armed = False
            stop_telemetry_collection()
            telemetry_deque = None

def rc_callback(msg):
    global failsafe_active
    with data_lock:
        channel_value = msg.channels[15] if len(msg.channels) > 15 else 0
        #rospy.loginfo('Channel 15 value: {}'.format(channel_value))
        if channel_value >= 1500 and is_armed and not failsafe_active:
            # Activate failsafe
            rospy.loginfo('Failsafe activated.')
            failsafe_active = True
            threading.Thread(target=failsafe_sequence).start()
        elif channel_value < 1500 and failsafe_active:
            # Deactivate failsafe
            rospy.loginfo('Failsafe deactivated.')
            failsafe_active = False

rospy.Subscriber('/mavros/state', State, state_callback)
rospy.Subscriber('/mavros/rc/in', RCIn, rc_callback)

rospy.spin()  # Spin forever
