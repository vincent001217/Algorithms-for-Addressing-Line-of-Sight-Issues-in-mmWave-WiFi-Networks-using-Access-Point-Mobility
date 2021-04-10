% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Results (number of STAs = 21)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all

X = categorical({'Heuristic (basic)','Heuristic (weighted)','ML solution'});
X = reordercats(X,{ 'Heuristic (basic)','Heuristic (weighted)','ML solution'});
Y = [0.718 0.8199 0.81;0.7572 0.7822 0.9469; 0.903 0.938 0.931]*100;
bar(X,Y); 
xlabel('Approach')
ylabel('LOS Prediction Accuracy (%)')
legend('Accuracy','Precision', 'Recall')
ylim([70 95])
grid on
