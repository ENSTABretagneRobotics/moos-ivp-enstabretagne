%%
% Conf

% Frame origin
lat0=42.954275;
lon0=10.601703;

%%%%%%%%%%%%%%% Log to KML
% Load the KML Toolbox
addpath('kmltoolbox v2.71/');
k = kml('KML');
%%
% Load data
% Files we are interested into

directory='klogs/';

Depth_file = 'KELLER_DEPTH.klog';

GPS_lat_file='GPS_LAT.klog';
GPS_lon_file='GPS_LON.klog';

%Dead_reck_X_file='Dead_reck_X.klog';
%Dead_reck_Y_file='Dead_reck_Y.klog';

%Loc_X_file='Loc_X.klog';
%Loc_Y_file='Loc_Y.klog';

fid=fopen([directory Depth_file]);
Depth=textscan(fid,'%f %*s %*s %f');
Depth_time=Depth{1};
Depth_data=Depth{2};
fclose(fid);

fid=fopen([directory GPS_lat_file]);
GPS_lat=textscan(fid,'%f %*s %*s %f');
GPS_lat_time=GPS_lat{1};
GPS_lat_data=GPS_lat{2};
fclose(fid);

fid=fopen([directory GPS_lon_file]);
GPS_lon=textscan(fid,'%f %*s %*s %f');
GPS_lon_time=GPS_lon{1};
GPS_lon_data=GPS_lon{2};
fclose(fid);

%%
% Interpolation

[r,ri]=unique(GPS_lat_data);
GPS_lat_I=interp1(GPS_lat_time(ri),r,Depth_time);
[r,ri]=unique(GPS_lon_data);
GPS_lon_I=interp1(GPS_lon_time(ri),r,Depth_time);

X=zeros(1,size(GPS_lon_I,1));
Y=zeros(1,size(GPS_lat_I,1));
[X,Y]=latLonToXY(GPS_lat_I,GPS_lon_I,lat0,lon0);

%%
% Plots
figure,plot(X,Y,'b--');
hold on;
% Dead Reck
% TODO
% Loc
% TODO
hold off;

%%
% Export KML
f = k.createFolder('dead_reck.plot');
t = linspace(0,360,1000);
f.plot(GPS_lon_data, GPS_lat_data, 'altitude', 0, 'altitudeMode','absolute','lineWidth',10, 'name','plot test');
k.run();