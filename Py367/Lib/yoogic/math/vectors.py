# -*- coding: utf-8 -*-
import math


def _raise_dim_error(dim1, dim2):
    raise ValueError("Vector Operands have %d != %d Dims!" % (dim1, dim2))


def _raise_type_error(desc, wanted, got):
    raise TypeError("%s requires a %s, got a %s: %s"
                    % (desc, wanted, type(got).__name__, str(got)))


def unpack_data(data):
    if isinstance(data, tuple):
        if len(data) != 2:
            _raise_dim_error(2, len(data))
        return data
    elif isinstance(data, Vector2):
        return data.x, data.y
    else:
        _raise_type_error("Vector2 Operation", "Vector2 or 2-Tuple", data)


class Vector2(object):

    def __init__(self, x=0.0, y=0.0):
        self.x = float(x)
        self.y = float(y)

    def __neg__(self):
        return Vector2(-self.x, -self.y)

    def __iadd__(self, another):
        dx, dy = unpack_data(another)
        self.x += dx
        self.y += dy
        return self

    def __isub__(self, another):
        dx, dy = unpack_data(another)
        self.x -= dx
        self.y -= dy
        return self

    def __imul__(self, scalar):
        self.x *= scalar
        self.y *= scalar
        return self

    def __mul__(self, scalar):
        return Vector2(self.x * scalar, self.y * scalar)

    def __add__(self, another):
        dx, dy = unpack_data(another)
        return Vector2(self.x + dx, self.y + dy)

    def __sub__(self, another):
        dx, dy = unpack_data(another)
        return Vector2(self.x - dx, self.y - dy)

    def __radd__(self, another):
        return self + another

    def __rsub__(self, another):
        return -(self - another)

    def __repr__(self):
        return "Vector2({0:.3f}, {1:.3f})".format(self.x, self.y)

    def __str__(self):
        return "({0:.3f}, {1:.3f})".format(self.x, self.y)

    def set_zero(self):
        self.x = 0.0
        self.y = 0.0
        return self

    def set_one(self):
        self.x = 1.0
        self.y = 1.0
        return self

    def set_unitx(self):
        self.x = 1.0
        self.y = 0.0
        return self

    def set_unity(self):
        self.x = 0.0
        self.y = 1.0
        return self

    def set_x(self, x):
        self.x = float(x)
        return self

    def set_y(self, y):
        self.y = float(y)
        return self

    def set_xy(self, x, y):
        self.x = float(x)
        self.y = float(y)
        return self

    def rotate(self, degrees):
        r = math.radians(degrees)
        c = math.cos(r)
        s = math.sin(r)
        x, y = self.x, self.y
        self.x = x * c - y * s
        self.y = x * s + y * c
        return self

    def rotated_by(self, degrees):
        return Vector2(self.x, self.y).rotate(degrees)

    def dot(self, another):
        ex, ey = unpack_data(another)
        return self.x * ex + self.y * ey

    def angle(self):
        m = self.length
        if m == 0:
            return 0.0
        c = self.x / m
        a = math.degrees(math.acos(c))
        return a if self.y >= 0 else 360.0 - a

    def angle_to(self, another):
        ex, ey = unpack_data(another)
        dot_r = self.x * ex + self.y * ey
        len_s = self.length
        len_a = math.sqrt(ex ** 2 + ey ** 2)
        cosa = dot_r / len_s / len_a
        return math.degrees(math.acos(cosa))

    @property
    def length(self):
        return math.sqrt(self.x ** 2 + self.y ** 2)

    def squared_distance(self, another):
        ex, ey = unpack_data(another)
        return (self.x - ex) ** 2 + (self.y - ey) ** 2

    def distance(self, another):
        return math.sqrt(self.squared_distance(another))
