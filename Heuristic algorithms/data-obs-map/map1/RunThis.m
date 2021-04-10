% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Heuristic algorithms (obstacle map 1)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all
numData = 10000;
M = 20;   % Number of existing STAs
P = 31;  % Number of discrete locations of the AP
thresh = 0.5;  % Threshold (<thresh: NLoS, >=thresh: LoS)
dist0 = 0.8;

LOS_ss = readNPY('los_sta_sta.npy');
LOS_as = readNPY('los_ap_sta.npy');
Truth_as = readNPY('los_ap_sta_wo_error.npy');

s_exist_STA = zeros(M+1,M);
a_exist_STA = zeros(P,M);
truth = zeros(P,1);
counter1 = 0;  % Basic
counter2 = 0;  % Weighted
counter1_perfect = 0;
counter2_perfect = 0;
counter_TP_FN = 0;
counter_TP_1 = 0;
counter_TP_FP_1 = 0;
counter_TP_2 = 0;
counter_TP_FP_2 = 0;

for i = 1:numData
    s_new_STA = double(reshape(LOS_ss(i,M+1,:),M+1,1));  % s_{M+1}
    for m = 1:M
        s_exist_STA(:,m) = double(reshape(LOS_ss(i,m,:),M+1,1));
        a_exist_STA(:,m) = double(reshape(LOS_as(i,:,m),P,1));
    end
    truth = double(reshape(Truth_as(i,:,M+1),P,1));
    dist = sum(abs(s_exist_STA-repmat(s_new_STA,1,M)));  % Hamming distance between the (M+1)th STA and the M existing STAs
    
    % Basic
    minValue = min(dist);
    V = find(dist == minValue);
    predict1 = sum(a_exist_STA(:,V),2)/length(V);
    predict1(predict1 < thresh) = 0;
    predict1(predict1 >= thresh) = 1;
    counter_TP_FN = counter_TP_FN+length(find(truth == 1));
    counter_TP_FP_1 = counter_TP_FP_1+length(find(predict1 == 1));
    counter_TP_1 = counter_TP_1+length(find(truth(find(predict1 == 1)) == 1));
    counter1 = counter1+(P-length(find(truth-predict1)));
    if(isempty(find(truth-predict1,1)))
        counter1_perfect = counter1_perfect+1;
    end
    
    % Weighted
    dist(dist == 0) = dist0;
    inv_dist = 1./dist;
    weight = inv_dist/sum(inv_dist);
    predict2 = sum(repmat(weight,P,1).*a_exist_STA,2);
    predict2(predict2 < thresh) = 0;
    predict2(predict2 >= thresh) = 1;
    counter_TP_FP_2 = counter_TP_FP_2+length(find(predict2 == 1));
    counter_TP_2 = counter_TP_2+length(find(truth(find(predict2 == 1)) == 1));
    counter2 = counter2+(P-length(find(truth-predict2)));
    if(isempty(find(truth-predict2,1)))
        counter2_perfect = counter2_perfect+1;
    end
end

Acc1 = counter1/P/numData
Prec1 = counter_TP_1/counter_TP_FP_1
Rec1 = counter_TP_1/counter_TP_FN
Acc2 = counter2/P/numData
Prec2 = counter_TP_2/counter_TP_FP_2
Rec2 = counter_TP_2/counter_TP_FN