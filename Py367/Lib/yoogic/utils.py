class ClassHooks(type):

    @staticmethod
    def __new__(cls, clsname, bases, dct):
        res = type.__new__(cls, clsname, bases, dct)
        for cls in bases:
            if isinstance(cls, SlotMeta):
                if hasattr(cls, 'extending'):
                    cls.extending(res)
        return res
