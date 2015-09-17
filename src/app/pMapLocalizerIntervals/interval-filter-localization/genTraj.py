import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PolyCollection
from scipy import interpolate


walls = [(70, 50), (0, 50), (0,0), (100, 0), (100, 100), (30, 100)]
poly = Polygon(walls,closed=False, fill=False)
plt.gca().add_patch(poly)
plt.axis([-10, 130, -10, 130])


WP = [(80, 120), (20, 100), (60, 70), (85, 50), (60, 20), (25, 10), (25, 35), (25, 40), (50, 30), (80, 20)]
x_WP = [ p[0] for p in WP ]
y_WP = [ p[1] for p in WP ]


tck,u = interpolate.splprep(np.array([x_WP, y_WP]), s=12)
dt = 1/(1200*25)
unew = np.arange(0, 1.00, 1/(1200*25))
out = interpolate.splev(unew, tck)
dout = interpolate.splev(unew, tck, 1)
plt.plot(out[0],out[1])
plt.figure();
#plt.plot(unew*1200, np.hypot(dout[0]/1200., dout[1]/1200))
plt.plot(np.hypot(dout[0]/1200., dout[1]/1200))
dx = (out[0][1:-1] - out[0][:-2])/0.04
dy = (out[1][1:-1] - out[1][:-2])/0.04
plt.plot( np.hypot(dx, dy))
#unew.flatten()[:-2],

with open('traj.txt','w') as f:
	for t,x,y,dx,dy in zip(unew*1200,out[0],out[1],dout[0], dout[1]):
		f.write(str(t)+ ' ' + str(x) + ' ' + str(y) + ' ' + str(np.hypot(dx/1200., dy/1200.)) + ' ' + str(np.arctan2(dy, dx)) + ' '+ '\n')

plt.show()
