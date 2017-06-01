#inspired by https://github.com/JKCooper2/gym-envs/blob/master/EightPuzzle/eight_puzzle.py
import logging
import gym
from gym import spaces
from gym.utils import seeding
import numpy as np
from six import StringIO
import sys

logger = logging.getLogger(__name__)

class VssAi(gym.Env):
    metadata = {
        'render.modes':['ansi','human'] #TODO trocar?
    }

    def __init__(self):
        self.size=15
        self.state = self._make_state()
        self.action_space = spaces.Discrete(10)
        self.observation_space = None #TODO achar space
        self.last_action = None
        self.last_reward = None
        self.seed()
        self.reset()
        self.viewer = None
        self.steps_beyond_done = None
        self._configure()
    def _make_state(self):
        pass #TODO retornar array de 15 elementos
    def _configure(self, display = None):
        self.display = display
    def _seed(self,seed=None):
        self.np_random,seed = seeding.np_random(seed) #TODO wtf
    def _step(self,action):
        pass
    def _reset(self):
        self.state = self._make_state()
        self.steps_beyond_done = None
        return self.state.flatten()