# -*- coding: utf-8 -*-
import bottle
from bottle import route, request, response, template

import random
import gym
import numpy as np
from collections import deque
from keras.models import Sequential
from keras.layers import Dense
from keras.layers.core import Activation
from keras.optimizers import RMSprop
from keras import backend as K

class DQNAgent():
    def __init__(self,state_size,action_size):
        self.state_size=state_size
        self.action_size=action_size
        self.memory = []
        for i in range(3):
            self.memory.append(deque(maxlen=100000))
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
    def _build_models(self):
        models = []
        for i in range(3):
            model = Sequential()
            model.add(Dense(6, init='lecun_uniform', input_shape=(14,)))
            model.add(Activation('relu'))
            model.add(Dense(self.action_size, init='lecun_uniform'))
            model.add(Activation('linear'))
            rms = RMSprop()
            model.compile(loss=self._huber_loss,optimizer=RMSprop(lr=self.learning_rate))
            models.append(model)
        return models
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
        self.mochange1dels[i].load_weights(name)
    def save(self,i,name):
        self.models[i].save_weights(name)




last_state = [[],[],[]]
last_actions = [-1,-1,-1]
last_state2 = [[],[],[]]
last_actions2 = [-1,-1,-1]
score=[0,0]
goal=0
#apenas mandar para a rede -- não pegar ação
@bottle.route('/send1', method="GET")
def receive1():
    global goal
    global score
    global last_state
    team = int(request.query.team)
    i = int(request.query.change)

    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    if team==1:
        if len(last_state[i])==0:
            last_state[i]=state
            return "first"
    else:
        if len(last_state2[i])==0:
            last_state2[i]=state
            return "first"
    action = float(request.query.action)
    #predicted_state,reward,done,_ = env.step(action)
    
    ball_x = state[12]
    done = False
    reward = 0
    if (ball_x>91) or (ball_x<9):
        done = True
        if team==1:
            if (ball_x>91):
                reward = 1
            else:
                reward = -1
        else:
            if ball_x<9:
                reward=1
            else:
                reward=-1

    state = np.reshape(state,[1,14])
    if team==1:
        agent.remember(i,last_state[i] ,last_actions[i],reward,state,done)
        last_state[i] = state
        last_actions[i] = action
    else:
        agent.remember(i,last_state2[i] ,last_actions2[i],reward,state,done)
        last_state2[i] = state
        last_actions2[i] = action
    
    return "ok"

#Recebe  a ação da rede neural -- diferença está no action
@bottle.route('/send2', method="GET")
def receive2():
    global goal
    global score
    global last_state
    team = int(request.query.team)
    i = int(request.query.change)

    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    
    if team==1:
        if len(last_state[i])==0:
            last_state[i]=state
            return str(random.randrange(agent.action_size))
    else:
        if len(last_state2[i])==0:
            last_state2[i]=state
            return str(random.randrange(agent.action_size))

    
    action = agent.act(i,state)
    
    ball_x = state[12]
    done = False
    reward = 0
    if (ball_x>91) or (ball_x<9):
        done = True
        if team==1:
            if (ball_x>91):
                reward = 1
            else:
                reward = -1
        else:
            if ball_x<9:
                reward=1
            else:
                reward=-1

    state = np.reshape(state,[1,14])
    if team==1:
        agent.remember(i,last_state[i] ,last_actions[i],reward,state,done)
        last_state[i] = state
        last_actions[i] = action
    else:
        agent.remember(i,last_state2[i] ,last_actions2[i],reward,state,done)
        last_state2[i] = state
        last_actions2[i] = action
    
    return str(action)

@bottle.route('/goal', method="GET")
def receive():
    team = request.query.team
    myteam = request.query.myteam
    if team==myteam:
        reward=1
    else:
        reward=-1
    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    
    for i in range(3):
        if team==1:
            if len(last_state[i])>0:
                agent.remember(i,last_state[i] ,last_actions[i],reward,state,True)
                last_state[i] = []
        else:
            if len(last_state2[i])>0:
                agent.remember(i,last_state2[i] ,last_actions2[i],reward,state,True)
                last_state2[i] = []
    return team

bottle.debug(True)
env = gym.make("VssAI-v0")
state_size = env.observation_space.n
action_size = env.action_space.n
agent = DQNAgent(state_size,action_size)
print(goal)
bottle.run(host='localhost', port=7777)
