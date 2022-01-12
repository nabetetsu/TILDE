#!/usr/bin/python3

import unittest

from rclpy.time import Time

from pathnode_vis.pub_info import (
    PubInfo,
    PubInfos
    )


def time_msg(sec, ms):
    """Get builtin.msg.Time"""
    return Time(seconds=sec, nanoseconds=ms * 10**6).to_msg()


class TestPubInfos(unittest.TestCase):
    def test_erase_until(self):
        infos = PubInfos()
        infos.add(PubInfo(
            "topic1",
            time_msg(8, 0),
            time_msg(9, 0),
            time_msg(10, 0)
        ))
        infos.add(PubInfo(
            "topic2",
            time_msg(18, 0),
            time_msg(19, 0),
            time_msg(20, 0)
        ))
        infos.add(PubInfo(
            "topic1",
            time_msg(28, 0),
            time_msg(29, 0),
            time_msg(30, 0)
        ))

        self.assertEqual(len(infos.topic_vs_pubinfos.keys()), 2)
        self.assertEqual(len(infos.topic_vs_pubinfos["topic1"].keys()), 2)
        self.assertEqual(len(infos.topic_vs_pubinfos["topic2"].keys()), 1)

        infos.erase_until(time_msg(9, 999))
        self.assertEqual(len(infos.topic_vs_pubinfos["topic1"].keys()), 2)
        self.assertEqual(len(infos.topic_vs_pubinfos["topic2"].keys()), 1)

        # boundary condition - not deleted
        infos.erase_until(time_msg(10, 0))
        self.assertEqual(len(infos.topic_vs_pubinfos["topic1"].keys()), 2)
        self.assertEqual(len(infos.topic_vs_pubinfos["topic2"].keys()), 1)

        infos.erase_until(time_msg(10, 1))
        self.assertEqual(len(infos.topic_vs_pubinfos["topic1"].keys()), 1)
        self.assertTrue("30.000000000" in
                        infos.topic_vs_pubinfos["topic1"].keys())
        self.assertEqual(len(infos.topic_vs_pubinfos["topic2"].keys()), 1)

        # topic2 at t=30.0 is deleted, but key is preserved
        infos.erase_until(time_msg(20, 1))
        self.assertEqual(len(infos.topic_vs_pubinfos.keys()), 2)
        self.assertEqual(len(infos.topic_vs_pubinfos["topic1"].keys()), 1)
        self.assertTrue("30.000000000" in
                        infos.topic_vs_pubinfos["topic1"].keys())
        self.assertEqual(len(infos.topic_vs_pubinfos["topic2"]), 0)

if __name__ == '__main__':
    unittest.main()