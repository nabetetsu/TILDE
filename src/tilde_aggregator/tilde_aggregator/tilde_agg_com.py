#!/usr/bin/env /usr/bin/python3
# -*- coding: utf-8 -*-
import sys, os, time
from pprint import pprint

from rclpy.clock import Clock, ClockType
from rclpy.time import Time

DEBUG_LEVEL_DEBUG = 1
DEBUG_LEVEL_INFO  = 2

g_debug_level = DEBUG_LEVEL_DEBUG
g_prev_time = 0.0
TILDE_AGG_DEBUG = 'TILDE_AGG_DEBUG'

DEBUG = False
g_msg_dump_enable = False

COMMAND_SHOW_INNER = 'show inner'
COMMAND_SHOW_STATIS = 'show statis'
COMMAND_SHOW_CLR_STATIS = 'showclr statis'
COMMAND_MSG_DUMP_ON = 'dump on'
COMMAND_MSG_DUMP_OFF = 'dump off'
COMMAND_DEBUG_ON = 'debug on'
COMMAND_DEBUG_OFF = 'debug off'


### for debug
import inspect
def location(depth=0):
    """Get execute loaction (file name & line number)

    Args:
        depth (int, optional): _description_. Defaults to 0.

    Returns:
        _type_: file name & line number
    """
    frame = inspect.currentframe().f_back
    return os.path.basename(frame.f_code.co_filename), frame.f_code.co_name, frame.f_lineno, '---'

def init_debug():
    """DEBUG enabling by environment variable 'export PRM_DEBUG=True'
    """
    global DEBUG

    v = os.getenv(TILDE_AGG_DEBUG, default=False)
    if type(v) is str and v.upper() == 'TRUE':
        DEBUG = True
    elif type(v) is bool and v == True:
        DEBUG = True
    else:
        DEBUG = False

def DP(dstr=None, level=DEBUG_LEVEL_DEBUG):
    """Debug print control

    Args:
        args: same print() arguments
    """
    if DEBUG:
        if level >= g_debug_level:
            global g_prev_time
            difftime = nano_to_sec(Clock(clock_type=ClockType.ROS_TIME).now()) - g_prev_time
            g_prev_time = nano_to_sec(Clock(clock_type=ClockType.ROS_TIME).now())
            if dstr is None or dstr == '' or len(dstr) == 0:
                print(f"[{difftime:.9f}]")
            else:
                print(f"[{difftime:.9f}] {dstr}")
            sys.stdout.flush()

def DP_INFO(dstr):
    """Info print control

    Args:
        args: same print() arguments
    """
    if DEBUG:
        if DEBUG_LEVEL_INFO >= g_debug_level:
            global g_prev_time
            difftime = nano_to_sec(Clock(clock_type=ClockType.ROS_TIME).now()) - g_prev_time
            g_prev_time = nano_to_sec(Clock(clock_type=ClockType.ROS_TIME).now())
            print(f"[{difftime:.9f}] {dstr}")
            sys.stdout.flush()

def stamp_to_sec(stamp):
    return stamp.sec + stamp.nanosec / (1000 * 1000 * 1000)

def nano_to_sec(t):
    return float(t.nanoseconds) / (1000.0 * 1000.0 * 1000.0)

###
def msg_dump(path_name, topic_name, msg, sub_time):
    global g_msg_dump_enable
    if g_msg_dump_enable == True:
        print(f"\n=== {sub_time:4f} [{topic_name}] in {path_name}: {msg.output_info.header_stamp} ===")
        for w in msg.input_infos:
            print(f"   (sub) [{w.topic_name}] {w.header_stamp}")

        sys.stdout.flush()
