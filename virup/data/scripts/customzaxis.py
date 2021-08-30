import numpy as np
from scipy.spatial.transform import Rotation as R


"""
# M31
z = np.array([-482.19478929, -90.9663576, -430.62306287])
inc=77.5
posangle=37.5
"""

# M33
z = np.array([-1136.20429226, -493.15169956, -734.27053424])
inc=-49
posangle=21.1


z /= np.linalg.norm(z)

uz=np.array([0.0, 0.0, 1.0])
up=np.cross(z, np.cross(uz, z))

r = R.from_rotvec(-np.pi*inc/180.0 * up)
z2 = np.matmul(r.as_matrix(), z)

r2 = R.from_rotvec(np.pi*posangle/180.0 * z)
z2 = np.matmul(r2.as_matrix(), z2)
print(z2)
