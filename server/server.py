# -*- coding: utf-8 -*-
import bottle
from bottle import route, request, response, template

import random
import gym
import numpy as np
from collections import deque
from keras.models import Sequential
from keras.layers import Dense
from keras.optimizers import RMSprop
from keras import backend as K

class DQNAgent():
    def __init__(self,state_size):
        self.state_size=state_size
        self.action_size=10
        self.memory = []
        for i in range(3):
            memory.append(deque(maxlen=100000))
        self.gamma = 0.9
        self.epsilon = 1.0
        self.e_decay = .99
        self.e_min = 0.05
        self.learning_rate = 0.0001
        self.models = self._build_models()
        self.target_models = self._build_models()
    def _huber_loss(self,target,prediction):
        error = prediction-target
        return K.mean(K.sqrt(1+K.square(error))-1,axis=1)
    def _build_model(self):
        for i in range(3):
            model = Sequential()
            model.add(Dense(6, init='lecun_uniform', input_shape=(14,)))
            model.add(Activation('relu'))
            model.add(Dense(self.action_size, init='lecun_uniform'))
            model.add(Activation('linear'))
            rms = RMSprop()
            model.compile(loss=self._huber_loss,optimizer=RMSprop(lr=self.learning_rate))
            self.models.append(model)
    def update_target_models(self):
        for i in range(3):
            self.target_models[i].set_weights(self.models[i].get_weights())
    def remember(self, i, state, action, reward, next_state, done):
        self.memory[i].append((state, action, reward, next_state, done))
    def act(self,i,state):
        if np.random.rand() <= self.epsilon:
            return random.randrange(self.action_size)
        act_values = self.models[i].predict(state)
        return np.argmax(act_values[0])
    def replay(self,i,batch_size):
        batch_size = min(batch_size,len(self.memory[i]))
        minibatch = random.sample(self.memory[i],batch_size)
        X = np.zeros((batch_size,self.state_size))#??
        Y = np.zeros((batch_size,self.action_size))
        for j in range(batch_size):
            state,action,reward,next_state,done = minibatch[j]
            target = self.models[i].predict(state)[0]
            if done:
                target[action] = reward
            else:
                a = np.argmax(self.models[i].predict(next_state)[0])
                t = self.target_models[i].predict(next_state)[0]
                target[action] = reward + self.gamma * t[a]
            X[i],Y[i] = state,target
        self.models[i].fit(X,Y,epochs=1,verbose=0)
        if self.epsilon > self.e_min:
            self.epsilon *= self.e_decay
    def load(self,i,name):
        self.models[i].load_weights(name)
    def save(self,i,name):
        self.models[i].save_weights(name)




@bottle.route('/send', method="GET")
def receive():

    '''foo = request.query.foo
    ballx = request.query.ballx
    bally = request.query.bally
    print(foo,ballx,bally)'''
    '''
    state = [request.query.l1x, request.query.l1y, request.query.l2x, request.query.l2y, request.query.l3x, request.query.l3y, request.query.r1x, request.query.r1y, request.query.r2x, request.query.r2y, request.query.r3x, request.query.r3y, request.query.ballx, request.query.bally]
    '''
    state=None #get state
    action = agent.act(state)
    predicted_state,reward,done,_ = env.step(action)
    predicted_state = np.reshape(next_state,[1,state_size])
    agent.remember(state,action,reward,predicted_state,done)
    
    return "ans"


bottle.debug(True)
env = gym.make("vssAI")
state_size = env.observation_space.shape[0]
action_size = env.acion_space.n
agent = DQNAgent(state_size,action_size)

bottle.run(host='localhost', port=7777)
