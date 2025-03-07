import vimr
import utils
from scipy.optimize import least_squares
from scipy.spatial.transform import Rotation as R
import matplotlib.pyplot as plt
import numpy as np
import cv2 as cv
import os
import csv
basis = np.array([
    [1,  0, 0, 0],
    [0, -1, 0, 0],
    [0,  0, 1, 0],
    [0,  0, 0, 1]])

absolute_path = os.path.dirname(__file__)
instance_path = os.path.join(absolute_path, '../vimr-instance')

[cams, board_dims, pts3d_w] = vimr.vimr_init(instance_path)

[init_poses_rav, init_pts3d_rav, pts2d] = utils.get_init_params(cams, pts3d_w)

init_params  = np.append(init_poses_rav, init_pts3d_rav)

num_points_total = len(pts3d_w) * len(cams[0]['img_files'])

def reshape_params(params):
  cam_params = params[:len(cams) * 6].reshape(len(cams), 6)
  pts3d = params[len(cams)*6:].reshape(num_points_total, 3)
  return [cam_params, pts3d]



def residual_func(params, draw=False):
    """Compute the residual reprojection error."""
    [cam_params, pts3d] = reshape_params(params)
    pts2_reproj = []
    for ci in range(len(cams)):
        R = cam_params[ci][:3]
        t = cam_params[ci][3:]
        [pts, jac] = cv.projectPoints(pts3d, R, t, cams[ci]['intrinsics'], None)
        pts2_reproj.extend(pts.ravel())
    return np.array(pts2d) - np.array(pts2_reproj)


result = least_squares(residual_func, init_params, verbose=2, x_scale='jac', ftol=1e-8)
[opt_poses, opt_pts3d] = reshape_params(result.x)
[init_pose, init_pts3d] = reshape_params(init_params)
print(init_pose)
print(opt_poses)

for i in range(len(opt_poses)):
  p = opt_poses[i]
  p_mat = utils.make_4dmat(p[:3], p[3:])
  cam_to_world = basis @ np.linalg.inv(p_mat) @ basis.T
  rotmat = cam_to_world[:3, :3]
  rot = R.from_matrix(rotmat)
  # This is x,y,z,w order but VIMR expects w,x,y,z
  q_xyzw = rot.as_quat().ravel()
  q_wxyz = [q_xyzw[3]]
  q_wxyz.extend(q_xyzw[:3])
  tvec = cam_to_world[:3, 3:]
  tvec[2] = tvec[2]
  pose = [0]
  pose.extend(tvec.ravel())
  pose.extend(q_wxyz)
  posefile = os.path.join(cams[i]['path'], cams[i]['id'] + ".world.pose")
  with open(posefile, 'w', encoding='UTF8') as f:
        writer = csv.writer(f, quoting=csv.QUOTE_NONE)
        writer.writerow(['{:.24f}'.format(x) for x in pose])
        print("saved {}".format(posefile))
  print(p_mat)