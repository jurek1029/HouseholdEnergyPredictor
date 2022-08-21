import numpy as np
import pandas as pd

vs = np.arange(5)
v = np.arange(5)
np.random.shuffle(vs)
d = np.column_stack((v,vs))
df = pd.DataFrame(d,columns=['v','vs'])
a = df.loc[1:len(df),'vs'].reset_index(drop=True)
b = df.loc[:len(df)-2,'vs'].reset_index(drop=True)
print(a -b)
print(df.loc[:,'vs'].diff(periods=1)) 

