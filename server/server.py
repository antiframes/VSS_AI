# -*- coding: utf-8 -*-
import bottle
from bottle import route, request, response, template

from firebase import firebase as firebase
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
        self.memory2 = []
        for i in range(3):
            self.memory.append(deque(maxlen=100000))
            self.memory2.append(deque(maxlen=100000))
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
            model.add(Dense(6, kernel_initializer='lecun_uniform', input_shape=(state_size,)))
            model.add(Activation('relu'))
            model.add(Dense(self.action_size, kernel_initializer='lecun_uniform'))
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
        X = np.zeros((batch_size,self.state_size))
        Y = np.zeros((batch_size,self.action_size))
        for j in range(batch_size):
            state,action,reward,next_state,done = minibatch[j]
            action=int(action)
            target = self.models[i].predict(state)[0]#TODO the list of Numpy arrays that you are passing to your model is not the size the model expected
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

#apenas mandar para a rede -- não pegar ação
@bottle.route('/send1', method="GET")
def receive1():
    global goal
    global score
    global last_state
    global env
    global timesteps
    team = int(request.query.team)
    i = int(request.query.change)
    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    if (env.unwrapped.is_null_state(i,team)):
        return "first"
    action = float(request.query.action)
    foo = env.unwrapped.show_last_state()[i][0][15:60]
    bar = np.reshape(state,(1,15))[0]
    env.unwrapped.set_data(i,team,state)
    final_state,reward,done,_ = env.step(action)
    last_state,last_action = env.unwrapped.get_last_state_and_action(team,i)
    agent.remember(i,last_state ,last_action,reward,final_state,done)
    agent.replay(0,3)
    return "ok"

#Recebe  a ação da rede neural -- diferença está no action
@bottle.route('/send2', method="GET")
def receive2():
    global goal
    global score
    global last_state
    global env
    team = int(request.query.team)
    i = int(request.query.change)
    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    if env.unwrapped.is_null_state(i,team):
        return str(random.randrange(agent.action_size))
    action = agent.act(i,state)
    env.unwrapped.set_data(i,team,state)
    final_state,reward,done,_ = env.step(action)
    last_state,last_action = env.unwrapped.get_last_state_and_action(team,i)
    agent.remember(i,last_state ,last_action,reward,final_state,done)  
    
    return str(action)

@bottle.route('/goal', method="GET")
def receive():
    team = request.query.team
    myteam = request.query.myteam
    state = [float(request.query.l1x), float(request.query.l1y), float(request.query.l2x), float(request.query.l2y), float(request.query.l3x), float(request.query.l3y), float(request.query.r1x), float(request.query.r1y), float(request.query.r2x), float(request.query.r2y), float(request.query.r3x), float(request.query.r3y), float(request.query.ballx), float(request.query.bally),team]
    reward,last_state,last_actions = env.unwrapped.step_goal(team,myteam)

    for i in range(3):
        agent.remember(i,last_state ,last_actions[i],reward,state,True)
    return team
fbase = firebase.FirebaseApplication('https://verysmallsize-eb3a1.firebaseio.com/', None)

@bottle.route('/end',method="GET")
def endgame():
    global fbase

    #salvar no banco de dados
    for i in range(len(agent.models)):
        agent.replay(i,128)
        model = agent.models[i]
        weights = model.get_weights()
        layer1 = np.transpose(weights[0]).tolist()
        layer2 = np.transpose(weights[2]).tolist()
        for j in range(6):
            fbase.put("/p"+str(i)+"/layer1",str(j) ,layer1[i])
        for j in range(10):
            fbase.put("/p"+str(i)+"/layer2",str(j) ,layer2[i])
    


bottle.debug(True)
env = gym.make("VssAI-v0")
env.reset()
state_size = env.observation_space.n
action_size = env.action_space.n
agent = DQNAgent(state_size,action_size)

#pegar do banco de dados
for i in range(3):
    layer1 = np.transpose(fbase.get("/p"+str(i)+'/layer1',None))
    layer2 = np.transpose(fbase.get("/p"+str(i)+'/layer2',None))
    weights = []
    weights.append(np.array(layer1))
    weights.append(np.zeros(6))
    weights.append(np.array(layer2))
    weights.append(np.zeros(10))
    agent.models[i].set_weights(np.array(weights))






bottle.run(host='localhost', port=7777)
