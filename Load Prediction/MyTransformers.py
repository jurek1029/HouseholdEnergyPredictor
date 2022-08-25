import pandas as pd
import numpy as np
from sklearn.base import BaseEstimator, TransformerMixin
from sklearn.preprocessing import StandardScaler

from sklearn.metrics import mean_squared_error, r2_score, mean_absolute_percentage_error,mean_absolute_error,median_absolute_error

class DiscreatDate(BaseEstimator, TransformerMixin):
    def __init__(self, date_tag='date',add_day=True, add_month=True, add_year=False, add_hour=False, add_min=False):
        self.date_tag = date_tag
        self.add_min = add_min
        self.add_hour = add_hour
        self.add_day = add_day
        self.add_month = add_month
        self.add_year = add_year
    def fit(self, X, y=None):
        return self  # nothing else to do
    def transform(self, X):
        X = X.copy()
        if self.add_min:
            X['minute'] = [date.minute for date in X[self.date_tag]]
        if self.add_day:
            X['hour'] = [date.hour for date in X[self.date_tag]]
        if self.add_day:
            X['day'] = [date.day for date in X[self.date_tag]]
        if self.add_month:
            X['month'] = [date.month for date in X[self.date_tag]]    
        if self.add_year:
            X['year'] = [date.year for date in X[self.date_tag]]
        return X

class AddDayPercentageFromDate(BaseEstimator, TransformerMixin):
    def __init__(self, date_tag='date'):
        self.date_tag = date_tag
    def fit(self, X, y=None):
        return self  # nothing else to do
    def transform(self, X):
        X = X.copy()
        X['dayPrecent'] = [(date.hour / 24 + date.minute / (60*24)) for date in X[self.date_tag]]
        return X

class AddDayPercentage(BaseEstimator, TransformerMixin):
    def __init__(self, hour_tag='hour', min_tag='minute', date_splited=True, date_tag='date'):
        self.hour_tag = hour_tag
        self.min_tag = min_tag
        self.date_tag = date_tag
        self.date_splited = date_splited
    def fit(self, X, y=None):
        return self  # nothing else to do
    def transform(self, X):
        X = X.copy()
        if self.date_tag:
            X['dayPrecent'] = X[self.hour_tag]/24 + X[self.min_tag]/(24*60)
        else :
            X['dayPrecent'] = X[self.date_tag].dt.hour/24 + X[self.date_tag].dt.minute/(24*60)
        return X

class AddYearPercentage(BaseEstimator, TransformerMixin):
    def __init__(self, month_tag='month', day_tag='day', date_splited=True, date_tag='date'):
        self.month_tag = month_tag
        self.day_tag = day_tag
        self.date_tag = date_tag
        self.date_splited = date_splited
    def fit(self, X, y=None):
        return self  # nothing else to do
    def transform(self, X):
        X = X.copy()
        if self.date_tag:
            X['yearPrecent'] = X[self.month_tag]/12 + X[self.day_tag]/(12*30) # month aproximation
        else:
            X['yearPrecent'] = X[self.date_tag].dt.month/12 + X[self.date_tag].dt.day/(12*30) # month aproximation
            
        return X

class WeatherDataMerge(BaseEstimator, TransformerMixin):
    def __init__(self, weather,on_tag='date', features=['temperatureMax']):
        self.on_tag = on_tag
        self.weather = weather
        self.features = features
    def fit(self, X, y=None):
        self.weather = self.weather[[self.on_tag]+self.features]
        return self
    def transform(self, X):
        X =  X.merge(self.weather,on=self.on_tag)
        return X

class HolidayDataMerge(BaseEstimator, TransformerMixin):
    def __init__(self, holiday, on_tag='date'):
        self.on_tag = on_tag
        self.holiday = holiday
    def fit(self, X, y=None):
        return self
    def transform(self, X):
        s = set(self.holiday['Bank holidays'])
        X['holiday_ind'] = [1 if date in s else 0 for date in X[self.on_tag]]
        return X
    
class Select(BaseEstimator, TransformerMixin):
    def __init__(self, features):
        self.features = features
    def fit(self, X, y=None):
        return self
    def transform(self, X):
        X = X.copy()
        return X[self.features]
    
class StdScaler(BaseEstimator, TransformerMixin):
    def __init__(self, features, added_past_data=0, data_tags=['avg_energy']):
        self.features = features
        self.added_past_data = added_past_data
        self.data_tags = data_tags
        self.stdScale = StandardScaler()
    def fit(self, X, y=None):
        while self.added_past_data > 0:
            self.added_past_data -= 1
            for tag in self.data_tags:
                self.features.append(f'{tag}_{self.added_past_data}')
        self.stdScale.fit(X[self.features])
        return self
    def transform(self, X):
        X = X.copy()
        X.loc[:,self.features] = self.stdScale.transform(X[self.features])
        return X
    
class AverageTransformer(BaseEstimator, TransformerMixin):
    def __init__(self, add_holiday=True): # no *args or **kargs
        self.add_holiday = add_holiday
    def fit(self, X, y=None):
        self.housecount = X.groupby('date')[['LCLid']].nunique()
        self.energy_sum = X.groupby('date')[['energy_sum']].sum()
        return self  # nothing else to do
    def transform(self, X):
        X = self.energy_sum.merge(self.housecount, on = ['date'])
        X.sort_values(by=['date'],inplace=True)
        X.reset_index(drop=False,inplace=True)
        X['avg_energy'] =  X['energy_sum']/X['LCLid']
        
        return X

class AddAvgDataFromPreviousDays(BaseEstimator, TransformerMixin):
    def __init__(self, days_back=7, data_tag='avg_energy'):
        self.days_back = days_back
        self.data_tag = data_tag
    def fit(self, X, y=None):
        return self
    def transform(self, X):
        X = X.copy()
        for index in range(0, self.days_back):
            X[f'{self.data_tag}_{index}'] = 0.0
            X.loc[self.days_back:len(X),f'{self.data_tag}_{index}'] = X.loc[self.days_back-index -1 :len(X)-index-2,self.data_tag].values
        X = X.iloc[self.days_back:,:]
        X.reset_index(drop=True,inplace=True)
        return X
    
class AddDiffOfAvgDataFromPreviousDays(BaseEstimator, TransformerMixin):
    def __init__(self, days_back=7, data_tag='avg_energy'):
        self.days_back = days_back
        self.data_tag = data_tag
    def fit(self, X, y=None):
        return self
    def transform(self, X):
        X = X.copy()
        for index in range(0, self.days_back):
            X[f'{self.data_tag}_diff_{index}'] = 0.0
            X.loc[1:,f'{self.data_tag}_diff_{index}'] =  X.loc[:,self.data_tag].diff(periods=index+1)[:-1].values
        X = X.iloc[self.days_back+1:,:]
        X.reset_index(drop=True,inplace=True)
        return X
    
class RemoveSmallDataHouseHolds(BaseEstimator, TransformerMixin):
    def __init__(self, samples_threshold=600, house_id_tag='LCLid'):
        self.samples_threshold = samples_threshold
        self.house_id_tag=house_id_tag
    def fit(self, X, y=None):
        self.households = X.groupby(self.house_id_tag).count()
        self.households = self.households[self.households['energy_sum'] < 600].index
        return self
    def transform(self, X):
        s = set(self.households)
        b = [e not in s for e in X[self.house_id_tag]]
        X = X[b]
        X.reset_index(drop=True,inplace=True)
        return X
    

    
def display_scores(scores):
    print("Scores:", scores)
    print("Mean:", scores.mean())
    print("Standard deviation:", scores.std())

def progres_bar(progress, total):
    precent = 100 * (progress / float(total))
    bar = '█' * int(precent) + '-' * int(100 - precent)
    print(f"\r|{bar}| {precent:.2f}%", end="\r")
    if precent == 100: print("")


def TestModelOnBatch(model,x_batches, y_test, funcs,mname=""):
    lenData = len(x_batches)

    test_preds = []
    score = []
    for i in range(0,lenData): ## Looping through test batches for making predictions
        preds = model.predict(x_batches[i])
        test_preds.extend(preds.tolist())
        progres_bar(i+1,lenData)

    for f in funcs:
        score.append(f(y_test, test_preds))
        print(f'{mname:>10} {f.__name__:>30} : {score[-1]}')
    return (test_preds,score)

def TestModel(model, data, lable, funcs, name):
    pred = model.predict(data)
    score = []
    for f in funcs:
        score.append(f(lable,pred))
        print(f'{name:>10} {f.__name__:>30} : {score[-1]}')
    return pred, score