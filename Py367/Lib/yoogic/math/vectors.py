# -*- coding: utf-8 -*-
import math


def _raise_dim_error(dim1, dim2):
    raise ValueError("Vector Operands have %d != %d Dims!" % (dim1, dim2))


def _raise_type_error(desc, wanted, got):
    raise TypeError("%s requires a %s, got a %s: %s"
                    % (desc, wanted, type(got).__name__, str(got)))


class Vector2(object):

    def __init__(self, x=0.0, y=0.0):
        self.x = float(x)
        self.y = float(y)

    def __neg__(self):
        return Vector2(-self.x, -self.y)

    def __iadd__(self, another):
        if isinstance(another, tuple):
            if len(another) != 2:
                _raise_dim_error(2, len(another))
            dx, dy = another
            self.x += dx
            self.y += dy
        elif isinstance(another, Vector2):
            self.x += another.x
            self.y += another.y
        else:
            _raise_type_error("Vector2 Add", "Vector2 or 2-Tuple", another)
        return self

    def __isub__(self, another):
        if isinstance(another, tuple):
            if len(another) != 2:
                _raise_dim_error(2, len(another))
            dx, dy = another
            self.x -= dx
            self.y -= dy
        elif isinstance(another, Vector2):
            self.x -= another.x
            self.y -= another.y
        else:
            _raise_type_error("Vector2 Sub", "Vector2 or 2-Tuple", another)
        return self

    def __add__(self, another):
        if isinstance(another, tuple):
            if len(another) != 2:
                _raise_dim_error(2, len(another))
            dx, dy = another
            return Vector2(self.x + dx, self.y + dy)
        elif isinstance(another, Vector2):
            return Vector2(self.x + another.x, self.y + another.y)
        else:
            _raise_type_error("Vector2 Add", "Vector2 or 2-Tuple", another)

    def __sub__(self, another):
        if isinstance(another, tuple):
            if len(another) != 2:
                _raise_dim_error(2, len(another))
            dx, dy = another
            return Vector2(self.x - dx, self.y - dy)
        elif isinstance(another, Vector2):
            return Vector2(self.x - another.x, self.y - another.y)
        else:
            _raise_type_error("Vector2 Add", "Vector2 or 2-Tuple", another)

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

    def squared_distance(self, another):
        if isinstance(another, tuple):
            if len(another) != 2:
                _raise_dim_error(2, len(another))
            ex, ey = another
        elif isinstance(another, Vector2):
            ex, ey = another.x, another.y
        else:
            _raise_type_error("Vector2 Dis2", "Vector2 or 2-Tuple", another)
        return (self.x - ex) ** 2 + (self.y - ey) ** 2

    def distance(self, another):
        return math.sqrt(self.squared_distance(another))
