# -*- coding: utf-8 -*-
from .. import utils


class FiniteStateMachine(object):

    def __init__(self, initial_state):
        self.current_state = initial_state
        self.states = list([initial_state])

    class State(object, metaclass=utils.ClassHooks):

        def extending(self):
            pass

        def on_register(self, state_machine):
            return

        async def is_valid(self):
            """
            StateInterface.is_valid
            ---------------
            | Indicate whether the state is currently valid.
            | If not, exceptions will be thrown if trying to transit to.
            """
            return True

        async def on_enter(self, data=None):
            return

        async def on_exit(self, data=None):
            return

        async def transition(self):
            raise NotImplementedError

    async def perform_transition(self):
        next_state = await self.current_state.transition()
        if next_state is not None:
            self.current_state = next_state
