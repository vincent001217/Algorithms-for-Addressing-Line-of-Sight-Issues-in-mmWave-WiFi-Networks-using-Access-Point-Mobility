% Paper: Algorithms for Addressing Line-of-Sight Issues in mmWave WiFi Networks using Access Point Mobility
% Results (ML in dynamic scenarios)
% Code Author: Ching-Lun Tai, ctai32@gatech.edu
% Updated Date: 2020.10.25
% All Rights Reserved

clear all

X = categorical({'100','500','1000','2000','5000','10000'});
X = reordercats(X,{ '100','500','1000','2000','5000','10000'});
Y = [0.837 0.95 0.874;0.847 0.941 0.866;0.886 0.925 0.913;0.89 0.933 0.92;0.892 0.933 0.915;0.903 0.938 0.931]*100;
bar(X,Y); 
xlabel('Timesteps used in training')
ylabel('LOS Prediction Accuracy (%)')
legend('Accuracy','Precision', 'Recall')
ylim([80 100])
grid on
