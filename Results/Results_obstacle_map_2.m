% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Results (obstacle map 2)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all

X = categorical({'Heuristic (basic)','Heuristic (weighted)','ML solution'});
X = reordercats(X,{ 'Heuristic (basic)','Heuristic (weighted)','ML solution'});
Y = [0.6768 0.76 0.7828;0.6937 0.7154 0.9296; 0.906 0.937 0.924]*100;
bar(X,Y); 
xlabel('Approach')
ylabel('LOS Prediction Accuracy (%)')
legend('Accuracy','Precision', 'Recall')
ylim([65 95])
grid on
