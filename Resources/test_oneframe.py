# -*- coding: utf-8 -*-
"""
Created on Sat Dec  8 14:34:36 2018

@author: 北海若
"""
import yookaic
frame = yookaic.begin_frame()
for i in range(100):
    yookaic.render_texture(frame, i + 100, i + 200, 200, 400)
yookaic.end_frame(frame)
