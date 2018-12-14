class Slot(object):

    def __init__(self):
        self.order_dirty = False
        self.handlers = list()

    def add_handler(self, handler, order=0):
        self.order_dirty = True
        self.handlers.append((order, handler))

    def remove_handler(self, handler):
        new_handlers = list()
        for order, cur_handler in self.handlers:
            if handler != cur_handler:
                new_handlers.append((order, cur_handler))
        self.handlers = new_handlers

    async def signal(self, data=None):
        if self.order_dirty:
            self.handlers.sort()
            self.order_dirty = False
        for _, handler in self.handlers:
            await handler(data)
