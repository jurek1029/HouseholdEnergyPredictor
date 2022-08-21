import os
import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow import keras
from matplotlib import pyplot as plt

from MyTransformers import *

mode_path = "Models/"

def rmspe(y_true, y_pred):
    loss = np.sqrt(np.mean(np.square(((y_true - y_pred) / y_true)), axis=0))
    return loss

def mape(y_true, y_pred): 
    return np.mean(np.abs((y_true - y_pred) / y_true)) * 100

def AgregateDiffs(xdiff, start=0):
    return (xdiff.cumsum() + start)
n=48
def plot_series(series, y=None, y_pred=None, _n=n, x_label="$t$", y_label="$x(t)$", legend=True):
    plt.plot(series, ".-")
    if y is not None:
        plt.plot(_n, y, "bo", label="Target")
    if y_pred is not None:
        plt.plot(_n, y_pred, "rx", markersize=10, label="Prediction")
    plt.grid(True)
    if x_label:
        plt.xlabel(x_label, fontsize=16)
    if y_label:
        plt.ylabel(y_label, fontsize=16, rotation=0)
    plt.hlines(0, 0, 100, linewidth=1)
    #plt.axis([0, _n + 1, -1, 1])
    if legend and (y or y_pred):
        plt.legend(fontsize=14, loc="upper left")

def plot_multiple_forecasts(X, Y, Y_pred, _n=n, toAgregate=False):
    n_steps = X.shape[1]
    ahead = Y.shape[1]
    if toAgregate:
        X = AgregateDiffs(X).reshape(-1,n_steps,1)
        Y = AgregateDiffs(Y,X[:,-1]).reshape(-1,ahead,1)
        Y_pred = AgregateDiffs(Y_pred,X[:,-1]).reshape(-1,ahead,1)

    plot_series(X[0, :, 0],_n=n)
    plt.plot(np.arange(n_steps, n_steps + ahead), Y[0, :, 0], "bo-", label="Actual")
    plt.plot(np.arange(n_steps, n_steps + ahead), Y_pred[0, :, 0], "rx-", label="Forecast", markersize=10)

    plt.axis([0, n_steps + ahead, X.min(), X.max()])
    plt.legend(fontsize=14)

def BatchWithNLen(n,pred_n,time_series, other_data=[],TVT_slit=(0.7,0.9)):
    if len(time_series) % n != 0:
        time_series = time_series[:len(time_series) - (len(time_series) % n)]

    dim = 1 if time_series.ndim == 1 else time_series.shape[-1]
    time_series = time_series.reshape(-1,n,dim)
    l = len(time_series)
    X_train = time_series[:int(l*TVT_slit[0])                  , :n-pred_n, 0].reshape(-1,n-pred_n,1)
    X_valid = time_series[int(l*TVT_slit[0]):int(l*TVT_slit[1]), :n-pred_n, 0].reshape(-1,n-pred_n,1) 
    X_test  = time_series[int(l*TVT_slit[1]):                  , :n-pred_n, 0].reshape(-1,n-pred_n,1)
    y_train = time_series[:int(l*TVT_slit[0])                  , -pred_n:, 0].reshape(-1,pred_n,1)
    y_valid = time_series[int(l*TVT_slit[0]):int(l*TVT_slit[1]), -pred_n:, 0].reshape(-1,pred_n,1)
    y_test  = time_series[int(l*TVT_slit[1]):                  , -pred_n:, 0].reshape(-1,pred_n,1)
    if len(other_data) > 0:
        if len(other_data) % n != 0:
           other_data = other_data[:len(other_data) - (len(other_data) % n)]

        dim = 1 if other_data.ndim == 1 else other_data.shape[-1]
        other_data = other_data.reshape(-1,n,dim)
        X_other_train = other_data[:int(l*TVT_slit[0])                  , -pred_n, :]
        X_other_valid = other_data[int(l*TVT_slit[0]):int(l*TVT_slit[1]), -pred_n, :]
        X_other_test  = other_data[int(l*TVT_slit[1]):                  , -pred_n, :]
        return (X_train, y_train, X_other_train), (X_valid, y_valid, X_other_valid) , (X_test, y_test, X_other_test)
    return (X_train, y_train), (X_valid, y_valid) , (X_test, y_test)
    
def OverlapingBatchWithNLen(n,pred_n,time_series, other_data=[],TVT_slit=(0.7,0.9)):
    if len(time_series) % n != 0:
        time_series = time_series[:len(time_series) - (len(time_series) % n)]

    dim = 1 if time_series.ndim == 1 else time_series.shape[-1]
    time_series = time_series.reshape(-1,n,dim)
    l = len(time_series)
    X_train = time_series[:int(l*TVT_slit[0])                  , :n-pred_n, 0].reshape(-1,n-pred_n,1)
    X_valid = time_series[int(l*TVT_slit[0]):int(l*TVT_slit[1]), :n-pred_n, 0].reshape(-1,n-pred_n,1) 
    X_test  = time_series[int(l*TVT_slit[1]):                  , :n-pred_n, 0].reshape(-1,n-pred_n,1)

    Y = np.empty((l, n-pred_n, pred_n))
    for step_ahead in range(1, pred_n + 1):
        Y[..., step_ahead - 1] = time_series[..., step_ahead:step_ahead + n - pred_n, 0]    
        
    y_train = Y[:int(l*TVT_slit[0])                  ]
    y_valid = Y[int(l*TVT_slit[0]):int(l*TVT_slit[1])]
    y_test  = Y[int(l*TVT_slit[1]):                  ]
    
    if len(other_data) > 0:
        if len(other_data) % n != 0:
           other_data = other_data[:len(other_data) - (len(other_data) % n)]

        dim = 1 if other_data.ndim == 1 else other_data.shape[-1]
        other_data = other_data.reshape(-1,n,dim)
        # X_other = np.empty((l, n-n_feature,n_feature, len(other_features)))
        # for step_ahead in range(1, pred_n + 1):
            # X_other[..., step_ahead - 1, :] = b[..., step_ahead:step_ahead + n - n_feature, 1:]
        # X_other_train = X_other[:int(l*0.7)           , ...]
        # X_other_valid = X_other[int(l*0.7):int(l*0.9) , ...]
        # X_other_test  = X_other[int(l*0.9):           , ...]

        X_other_train = other_data[:int(l*TVT_slit[0])                  , pred_n:n, :]
        X_other_valid = other_data[int(l*TVT_slit[0]):int(l*TVT_slit[1]), pred_n:n, :]
        X_other_test  = other_data[int(l*TVT_slit[1]):                  , pred_n:n, :]
        return (X_train, y_train, X_other_train), (X_valid, y_valid, X_other_valid) , (X_test, y_test, X_other_test)
    return (X_train, y_train), (X_valid, y_valid) , (X_test, y_test)
    
class MyLoader:
    def __init__(self,files,batchSize,timeSeriesLables, otherDataLabels,splitFun, sumInN = 1, asDiff = False, asSum = False, TVT_split=(0.7,0.9),
     pred_n = 10,lable="default"):
        self.batchSize = batchSize
        self.timeSeriesLables = timeSeriesLables
        self.otherDataLabels = otherDataLabels
        self.sumInN = sumInN
        self.asDiff = asDiff
        self.asSum = asSum
        self.files = files
        self.TVT_split = TVT_split
        self.pred_n = pred_n
        self.splitFun = splitFun
        self.lable = lable

    def loadDataTimeSeries(files, features, batchSize,sum_n = 1, asSum=True, asDiff=True):
        tmpArr = [[]]*len(files)
        dataOut = np.empty((0,len(features)))
        for i,file in enumerate(files):
            progres_bar(i + 1,len(files))
            data = pd.read_feather(file)
            data = data[features].values
            if asSum:
                data = data[:len(data) - (len(data) % sum_n)]
                if len(features) > 1:
                    data = data.reshape(-1,sum_n,len(features))     
                else:
                    data = data.reshape(-1,sum_n)
                data = data.sum(axis=1,keepdims=True)
            if asDiff:
                data = np.diff(data,axis=0)
                data = data[1:]

            tmpArr[i] = data[:len(data) - (len(data) % batchSize)]
        if len(files) > 0:    
            dataOut = np.row_stack(tmpArr)
        else:
            dataOut = tmpArr[0]
        return dataOut.astype('f4')

    def loadDataOther(files, features, batchSize):
        tmpArr = [[]]*len(files)
        dataOut = np.empty((0,len(features)))
        for i,file in enumerate(files):
            progres_bar(i + 1,len(files))
            data = pd.read_feather(file)
            data = data[features].values
            tmpArr[i] = data[:len(data) - (len(data) % batchSize)]
        if len(files) > 0:    
            dataOut = np.row_stack(tmpArr)
        else:
            dataOut = tmpArr[0]
        return dataOut.astype('f4')

    def load(self):
        self.timeSeries = MyLoader.loadDataTimeSeries(self.files, self.timeSeriesLables, self.batchSize, self.sumInN, self.asSum, self.asDiff)
        if self.hasOther():
            self.otherData = MyLoader.loadDataOther(self.files, self.otherDataLabels, self.batchSize)
            (self.XTrain, self.YTrain, self.XOtherTrain), (self.XValid, self.YValid, self.XOtherValid), (self.XTest, self.YTest, self.XOtherTest) = self.splitFun(self.batchSize,self.pred_n,self.timeSeries, self.otherData, self.TVT_split)
        else:
            (self.XTrain, self.YTrain), (self.XValid, self.YValid), (self.XTest, self.YTest) = self.splitFun(self.batchSize,self.pred_n,self.timeSeries, [], self.TVT_split)

    def hasOther(self):
        return True if self.otherDataLabels else False
    
    def setArgs(self,args):
        self.args = args

class MyModel:
    _model = None
    _lableDefined = False
    _dataLoaded = False
    _trainingStarted = False
    _epoch = 0
    def __init__(self, batchSize, loader,lable,createModel,modelArgs,modleDims,predDataFun):
        self.batchSize = batchSize
        self.loader = loader
        self._lable = lable
        self.createModel = createModel
        self.modelArgs = modelArgs
        self.modleDims = modleDims
        self.predDataFun = predDataFun

    def load(self):
        if not self._dataLoaded:
            self.loader.load() 
            self._dataLoaded = True
        
    def model(self):
        if self._model == None:
            dim = 1 if self.loader.timeSeries.ndim == 1 else self.loader.timeSeries.shape[-1]
            if self.loader.hasOther():
                dim = [dim, 1 if self.loader.otherData.ndim == 1 else self.loader.otherData.shape[-1]]
            self.loader.setArgs(self.modelArgs)
            self._model = self.createModel(self.batchSize,dim,self.modelArgs, self.modleDims)
        return self._model

    def lable(self):
        if not self._lableDefined:
            for i in range(len(self.modleDims)):
                self._lable += f"-{self.modleDims[i]}" 
            self._lable += '-Batch-'+ str(self.batchSize)
            self._lableDefined = True
        return self._lable

    def prepModel(self, loss='mse', optimizer='adam', learningRate=0.01):
        if optimizer == 'adam':
            opt = keras.optimizers.Adam(learning_rate=learningRate)
        self.model().compile(loss=loss, optimizer=opt)
        self.tbCallBack = keras.callbacks.TensorBoard(log_dir=f'./Graph/{self.loader.lable}/n{self.batchSize}/{optimizer}-LeRate-{learningRate}-loss-{loss}/{self.lable()}', histogram_freq=0, write_graph=True, write_images=True)
        self.earlyStopCallback = tf.keras.callbacks.EarlyStopping(monitor='loss', patience=20,min_delta=0, restore_best_weights=True)

    def train(self,epochs,loss='mse', optimizer='adam', learningRate=0.01, batch_size=4, isEarlyStoping=True, isSaveTensorBoard=True,**kwargs):
        if not self._trainingStarted:
            self.load()
            self.prepModel(loss,optimizer,learningRate)
            self.history = []
            self.trainData , self.validData, self.testData = self.predDataFun(self.loader)
            self._trainingStarted = True

        callbacks = []
        if isSaveTensorBoard: callbacks.append(self.tbCallBack)
        if isEarlyStoping: callbacks.append(self.earlyStopCallback) 

        self.history.append(self.model().fit(self.trainData[0], self.trainData[1], epochs=epochs,batch_size=batch_size,
        validation_data=self.validData,callbacks=callbacks,**kwargs))
        print("Done Fiting.")
        self._epoch += epochs
    

    def test(self,plotsCount=5,batchStart=1,random=False):
        batchs = range(batchStart,batchStart + plotsCount)
        if random:
            batchs = np.round(np.random.rand(plotsCount) * (len(self.testData[1])-1)).astype('i4')
        for batch in batchs:
            print(batch)
            if self.loader.hasOther():
                X_new = [[]]*len(self.testData[0])
                for i,vec in enumerate(self.testData[0]):
                    X_new[i] = vec[batch:batch+1]
            else:
                X_new = self.testData[0][0][batch:batch+1]

            Y_new = self.testData[1][batch:batch+1]
            if self.loader.splitFun == OverlapingBatchWithNLen:
                Y_pred = self.model().predict(X_new)[:, -self.loader.pred_n:]
                Y_new = Y_new[:, -self.loader.pred_n:]
            elif self.loader.splitFun == BatchWithNLen:
                Y_pred = self.model().predict(X_new).reshape(1,self.loader.pred_n,1)
            else: print("what")
            if self.loader.hasOther():
                plot_multiple_forecasts(X_new[0], Y_new, Y_pred,_n=self.batchSize, toAgregate=self.loader.asDiff)
            else:
                plot_multiple_forecasts(X_new, Y_new, Y_pred,_n=self.batchSize, toAgregate=self.loader.asDiff)
            plt.figure()

    def save(self):
        self.model().save(mode_path + f'{self.lable()}-epoch-{self._epoch}')

    def testResult(self, show=False):
        l = len(self.testData[1])
        l_pSum = np.zeros(l); l_ySum = np.zeros(l); l_diff = np.zeros(l); l_prec = np.zeros(l); l_mse = np.zeros(l)
        l_rmspe = np.zeros(l); l_mape = np.zeros(l); l_Y = np.zeros((l,self.loader.pred_n)); l_pred = np.zeros((l,self.loader.pred_n))
        for j in range(l):
            if self.loader.hasOther():
                X = [[]]*len(self.testData[0])
                for i,vec in enumerate(self.testData[0]):
                    X[i] = vec[j:j+1]
            else:
                X = self.testData[0][0][j:j+1]
            Y = self.testData[1][j:j+1]

            if self.loader.splitFun == OverlapingBatchWithNLen:
                pred = self.model().predict(X)[:, -1:].reshape(self.loader.pred_n)
                Y = Y[-1,-1]
            elif self.loader.splitFun == BatchWithNLen:
                pred = self.model().predict(X).reshape(self.loader.pred_n)
                Y = Y.reshape(self.loader.pred_n)
            else: 
                print("what")
                print(self.loader.splitFun,OverlapingBatchWithNLen,BatchWithNLen)
            if self.loader.asDiff:
                pred = pred.cumsum()
                Y = Y.cumsum()
            l_pSum[j] = pred.sum()
            l_ySum[j] = Y.sum()
            l_diff[j] = l_ySum[j] - l_pSum[j]
            l_prec[j] = ((l_diff[j] / l_ySum[j]) * 100)
            l_mse[j] = (mean_squared_error(Y,pred))
            l_rmspe[j] = rmspe(Y,pred)
            l_mape[j] = mape(Y,pred)
            l_Y[j] = Y
            l_pred[j] = pred.copy()
            if show:
                print(f"predSum: {l_pSum[j]: .4f} YSum: {l_ySum[j]: .4f} diff: {l_diff[j]: .4f} precent: {l_prec[j]: .4f}% \t mse: {l_mse[j]: .4f} rmspe: {l_rmspe[j]: .4f} mape: {l_mape[j]: .4f}") 
        return l_pSum, l_ySum, l_diff, l_prec, l_mse, l_rmspe, l_mape, l_Y, l_pred


def createFlattenModel(batchSize,timeSDim,args,dims): #batchSize, Dims
          return keras.models.Sequential([
            keras.layers.Flatten(input_shape=[batchSize-dims[-1], timeSDim]),
            keras.layers.Dense(dims[-1])])
def predDataDefault(l):
    return ([l.XTrain],l.YTrain),([l.XValid], l.YValid),[[l.XTest],l.YTest]
        
def predDataConv(l):
    return ([l.XTrain],l.YTrain[:,(l.args[0]-1)::l.args[1]]),([l.XValid], l.YValid[:,(l.args[0]-1)::l.args[1]]),[[l.XTest],l.YTest[:,(l.args[0]-1)::l.args[1]]]
        
def predDataWithOther(l):
    return ([l.XTrain,l.XOtherTrain],l.YTrain),([l.XValid,l.XOtherValid], l.YValid),[[l.XTest,l.XOtherTest],l.YTest]

    X_train, Y_train[:,(k_size-1)::stride]
        
def createSimpleRNN (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.SimpleRNN(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.SimpleRNN(dims[i], return_sequences=True)) for i in range(1,len(dims)-1)]
    m.add(keras.layers.SimpleRNN(dims[-1]))
    return m

def createSimpleRNNTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.SimpleRNN(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.SimpleRNN(dims[i], return_sequences=True)) for i in range(1,len(dims)-1)]
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createSimpleRNNWithOtherData(batchSize, dataDim, args, dims):
    input_A = keras.layers.Input(shape=[None, dataDim[0]], name="Time-series")
    input_B = keras.layers.Input(shape=[dataDim[1]], name="other")
    hidden1 = keras.layers.SimpleRNN(dims[0], return_sequences=True)(input_A)
    for i in range(1,len(dims)-2):
        hidden1 = keras.layers.SimpleRNN(dims[i], return_sequences=True)(hidden1)
    hidden2 = keras.layers.SimpleRNN(dims[-2], return_sequences=False)(hidden1)
    hidden3 = keras.layers.Dense(dims[-1])(hidden2)
    concat = keras.layers.concatenate([input_B, hidden3])
    output = keras.layers.Dense(dims[-1], name="output")(concat)
    return keras.Model(inputs=[input_A, input_B], outputs=[output])

def createLSTMWithOtherData(batchSize, dataDim, args, dims):
    input_A = keras.layers.Input(shape=[None, dataDim[0]], name="Time-series")
    input_B = keras.layers.Input(shape=[None,dataDim[1]], name="other")
    hidden1 = keras.layers.LSTM(dims[0], return_sequences=True)(input_A)
    for i in range(1,len(dims)-2):
        hidden1 = keras.layers.LSTM(dims[i], return_sequences=True)(hidden1)
    hidden2 = keras.layers.LSTM(20, return_sequences=True)(hidden1)
    hidden3 = keras.layers.TimeDistributed(keras.layers.Dense(dims[-1]))(hidden2)
    hidden4 = keras.layers.TimeDistributed(keras.layers.Dense(dims[-1]))(input_B)
    concat  = keras.layers.concatenate([hidden4, hidden3],axis=2)
    output  = keras.layers.Dense(1, name="output")(concat)
    return keras.Model(inputs=[input_A, input_B], outputs=[output])

def createSimpleRNNWithBatchNorm(batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.SimpleRNN(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    for i in range(1,len(dims)-1):
        m.add(keras.layers.SimpleRNN(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.SimpleRNN(dims[-1]))
    return m

def createSimpleRNNWithBatchNormTimeDist(batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.SimpleRNN(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    for i in range(1,len(dims)-1):
        m.add(keras.layers.SimpleRNN(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createLSTM (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.LSTM(dims[i], return_sequences=True)) for i in range(1,len(dims)-2)]
    m.add(keras.layers.LSTM(dims[-2], return_sequences=False))
    m.add(keras.layers.Dense(dims[-1]))
    return m

def createLSTMTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.LSTM(dims[i], return_sequences=True)) for i in range(1,len(dims)-1)]
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createGRU (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.GRU(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.GRU(dims[i], return_sequences=True)) for i in range(1,len(dims)-1)]
    m.add(keras.layers.Dense(dims[-1]))
    return m

def createGRUTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.GRU(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    [m.add(keras.layers.GRU(dims[i], return_sequences=True)) for i in range(1,len(dims)-1)]
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createConvGRU (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.Conv1D(filters=dims[0], kernel_size=args[0], strides=args[1], padding="valid",input_shape=[None, timeSDim]))
    [m.add(keras.layers.GRU(dims[i], return_sequences=True)) for i in range(0,len(dims)-1)]
    m.add(keras.layers.Dense(dims[-1]))
    return m

def createConvGRUTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.Conv1D(filters=dims[0], kernel_size=args[0], strides=args[1], padding="valid",input_shape=[None, timeSDim]))
    [m.add(keras.layers.GRU(dims[i], return_sequences=True)) for i in range(0,len(dims)-1)]
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createLSTMBatchNormDrop (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.Dropout(rate=args[0]))
    for i in range(1,len(dims)-2):
        m.add(keras.layers.LSTM(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
        m.add(keras.layers.Dropout(rate=args[0]))
    m.add(keras.layers.LSTM(dims[-2], return_sequences=False))
    m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.Dropout(rate=args[0]))
    m.add(keras.layers.Dense(dims[-1]))
    return m

def createLSTMBatchNormDropTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.Dropout(rate=args[0]))
    for i in range(1,len(dims)-1):
        m.add(keras.layers.LSTM(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
        m.add(keras.layers.Dropout(rate=args[0]))
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createLSTMBatchNorm (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    for i in range(1,len(dims)-2):
        m.add(keras.layers.LSTM(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.LSTM(dims[-2], return_sequences=False))
    m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.Dense(dims[-1]))
    return m

def createLSTMBatchNormTimeDist (batchSize, timeSDim, args, dims): 
    m = keras.models.Sequential()
    m.add(keras.layers.LSTM(dims[0], return_sequences=True, input_shape=[None, timeSDim]))
    m.add(keras.layers.BatchNormalization())
    for i in range(1,len(dims)-1):
        m.add(keras.layers.LSTM(dims[i], return_sequences=True))
        m.add(keras.layers.BatchNormalization())
    m.add(keras.layers.TimeDistributed(keras.layers.Dense(dims[-1])))
    return m

def createWaveNet (batchSize, timeSDim, args=[20,4,2,2,'relu'], dims=[10]): 
    m = keras.models.Sequential()
    m.add(keras.layers.InputLayer(input_shape=[None, timeSDim]))
    for rate in [2**i for i in range(args[1])] * args[2]:
        m.add(keras.layers.Conv1D(filters=args[0], kernel_size=args[3], padding="causal", activation=args[4], dilation_rate=rate))
    m.add(keras.layers.Conv1D(filters=dims[-1], kernel_size=1))
    return m
