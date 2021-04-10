% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Results (obstacle map 1)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all

X = categorical({'Heuristic (basic)','Heuristic (weighted)','ML solution'});
X = reordercats(X,{ 'Heuristic (basic)','Heuristic (weighted)','ML solution'});
Y = [0.6955 0.7871 0.8062;0.7307 0.7532 0.9454; 0.9 0.94 0.923]*100;
bar(X,Y); 
xlabel('Approach')
ylabel('LOS Prediction Accuracy (%)')
legend('Accuracy','Precision', 'Recall')
ylim([65 95])
grid on
