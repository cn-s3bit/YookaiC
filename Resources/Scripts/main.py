# -*- coding: utf-8 -*-
"""
Created on Sat Dec  8 17:13:25 2018

@author: 北海若
"""
import yookaic
import asyncio
import time


async def main():
    texture_id = yookaic.load_texture2d(
                    "../Resources/Game/Image/Ming/Ming.png"
                 )
    pos_x = 0.0
    pos_y = 0.0
    rot = 0.0
    scl = 1.0
    vel_x = 0.0
    vel_y = 0.0
    vel_rot = 0.0
    vel_scl = 0.0
    while True:
        begin_t = time.time()
        event = yookaic.poll_event()
        if event is not None:
            print(event)
            if event[0] == 32767:
                break
            if event[0] in (1, 2):
                if event[1] == (1 << 30) | 79:
                    vel_x -= 12.0 * (event[0] - 1.5)
                elif event[1] == (1 << 30) | 80:
                    vel_x += 12.0 * (event[0] - 1.5)
                elif event[1] == (1 << 30) | 81:
                    vel_y -= 12.0 * (event[0] - 1.5)
                elif event[1] == (1 << 30) | 82:
                    vel_y += 12.0 * (event[0] - 1.5)
                elif event[1] == ord('q'):
                    vel_rot += 1.0 * (event[0] - 1.5)
                elif event[1] == ord('e'):
                    vel_rot -= 1.0 * (event[0] - 1.5)
                elif event[1] == ord('u'):
                    vel_scl += 0.02 * (event[0] - 1.5)
                elif event[1] == ord('i'):
                    vel_scl -= 0.02 * (event[0] - 1.5)
        pos_x += vel_x
        pos_y += vel_y
        rot += vel_rot
        scl *= 1.0 + vel_scl
        frame = yookaic.begin_frame()
        yookaic.bind_texture2d(frame, texture_id)
        yookaic.render_texture_ex(frame, pos_x, pos_y, 0.5, 0.5, rot, scl, scl)
        yookaic.end_frame(frame)
        wait_time = 1.0 / 60.0 - time.time() + begin_t
        if wait_time > 0.0:
            await asyncio.sleep(wait_time)
    yookaic.dispose_texture2d(texture_id)


asyncio.get_event_loop().run_until_complete(main())
