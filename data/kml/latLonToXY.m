function [ x,y ] = latLonToXY( lat,lon,lat0,lon0)
    x=6371000*cos(lat0*pi/180)*(lon-lon0)*pi/180;
    y=6371000*(lat-lat0)*pi/180;
end

