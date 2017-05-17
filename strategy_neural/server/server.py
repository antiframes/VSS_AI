import bottle,os,pprint
from firebase import firebase as firebase
from keras.models import Sequential
from keras.layers.core import Dense, Dropout, Activation
from keras.optimizers import RMSprop
import numpy as np
import io



@bottle.route('/upload',method="POST")
def receiveLog():
    upload = bottle.request.files.get('upload')
    name, ext = os.path.splitext(upload.filename)
    if ext !=".txt":
        return 'Envie um .txt.'
    f = str(upload.file.read())[2:]
    
    #f = upload.file.getvalue()
    
    
    models = []
    for i in range(3):
        model = Sequential()
        model.add(Dense(6, init='lecun_uniform', input_shape=(14,)))
        model.add(Activation('relu'))
        model.add(Dense(10, init='lecun_uniform'))
        model.add(Activation('linear'))
        rms = RMSprop()
        model.compile(loss='mse', optimizer=rms)
        models.append(model)
    
    
    lines = f.split("\\n")
    for state in lines:
        
        #converter string em lista de floats
        state = state.split(" ")
        if (len(state)!=17):
                break
        for i in range(len(state)):
            state[i]=float(state[i])
            
            
        
        input_data = np.array(state[1:15],dtype=np.float32)
        action = state[15]
        reward = state[16]
        
        qval = model.predict(input_data.reshape(1,14), batch_size=1)
        
        
        y = np.zeros((1,10))
        y[0][int(action)] = reward
        models[int(state[0])].fit(input_data.reshape(1,14), y, batch_size=1, nb_epoch=1, verbose=1)
    
    fbase = firebase.FirebaseApplication('https://verysmallsize-eb3a1.firebaseio.com/', None)
    
    for i in range(3):
        weights = models[i].get_weights()
        layer1 = np.transpose(weights[0]).tolist()
        layer2 = np.transpose(weights[2]).tolist()
        for j in range(6):
            fbase.put("/p"+str(i)+"/layer1",str(j) ,layer1[i])
        for j in range(10):
            fbase.put("/p"+str(i)+"/layer2",str(j) ,layer2[i])
    return "Feito!"
    

    


@bottle.route("/")
def index():
    return '<form action="/upload" method="post" enctype="multipart/form-data">Arquivo de log: <br><input type="file" name="upload" /><br><input type="submit" value="Enviar" /></form>'

bottle.debug(True)
bottle.run(host='localhost', port=7777)
