% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Results (number of STAs = 11)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all

X = categorical({'Heuristic (basic)','Heuristic (weighted)','ML solution'});
X = reordercats(X,{ 'Heuristic (basic)','Heuristic (weighted)','ML solution'});
Y = [0.6545 0.7222 0.7989;0.6586 0.7001 0.8706; 0.914 0.937 0.933]*100;
bar(X,Y); 
xlabel('Approach')
ylabel('LOS Prediction Accuracy (%)')
legend('Accuracy','Precision', 'Recall')
ylim([65 95])
grid on
