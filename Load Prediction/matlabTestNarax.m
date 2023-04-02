n = 120;
toPred = 48;
data = squeeze(sum(reshape(SumProfiles_Electricity.Sum_kWh_,[],n),2));
test = data(size(data,1)-toPred:size(data,1));
data = data(1:size(data,1)-toPred);

data = data.';
data = num2cell(data);

net = narnet(1:2,[10,5]);
[Xs,Xi,Ai,Ts] = preparets(net,{},{},data);
net = train(net,Xs,Ts,Xi,Ai);
%view(net)
[Y,Xf,Af] = net(Xs,Xi,Ai);
perf = perform(net,Ts,Y)
[netc,Xic,Aic] = closeloop(net,Xf,Af);
Yc = netc(cell(0,toPred),Xic,Aic)
Yc = cell2mat(Yc)
mseVal = mse(test,Yc)
plot(Yc)
hold on 
plot(test)


%mseVal = 0.6569