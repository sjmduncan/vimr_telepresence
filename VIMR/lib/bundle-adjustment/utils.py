import cv2 as cv

import numpy as np
import os


def make_4dmat(r, t):

    """Combine OpenCVs  r and t into 4D transform matrix."""

    bottomrow = np.array([np.zeros(4)])

    bottomrow[0, 3] = 1

    [rmat, jac] = cv.Rodrigues(r.ravel())

    # Make sure translation is a column vector

    tvec = np.array([t.ravel()]).T

    return np.concatenate((np.concatenate((rmat, tvec), axis=1), bottomrow))



def relative_transform(r0, t0, r, t):

    """

    Compute transform to align points in frame of [r|t] to [r0|t0].


    Parameters

    ----------

    r0 : array

        Rodriguez orientation of the first ref frame

    t0 : array

        Translation vector of the first ref frame

    r : array

        Rodriguez orientation of the second ref frame

    t : array

        Translation vector of the first ref frame



    Returns

    -------

    list : [Rc, tc]

        The 3x3 rotation matrix and 3x1 (column) translation vector which will

        bring points from the second ref frame to the first

    """

    A0 = make_4dmat(r0, t0)

    A0inv = np.linalg.inv(A0)

    A1 = make_4dmat(r, t)

    Ac = A0inv @ A1

    Rc = Ac[:3, :3]

    tc = Ac[:3, 3:]

    return [Rc, tc]


def find_board(impath, board_dims, intrinsics, pts3d):

  spcrit = (cv.TermCriteria_EPS, 30, 0.01)

  im = cv.imread(impath)
  corners = []
  r = []
  t = []

  posed = False

  [found, corners] = cv.findChessboardCorners(im, board_dims)

  if found:

    im_gray = cv.cvtColor(im, cv.COLOR_RGB2GRAY)

    corners_refined = cv.cornerSubPix(im_gray, corners, [5, 5], [-1, -1], spcrit)

    [posed, r, t] = list(cv.solvePnP(pts3d, corners_refined, intrinsics, None))

  cv.drawChessboardCorners(im, board_dims, corners, found)

  if posed:

    cv.drawFrameAxes(im, intrinsics, None, r, t, 0.3)

  #cv.imshow("Pose", im)

  print("{} {} {}".format(found, posed, impath))

  #cv.waitKey(100)

  return [posed, corners, r, t]


def get_init_params(cams, pts3d):
  init_poses = []
  init_pts3d = []
  pts2d = []
  for p0 in pts3d:
      init_pts3d.extend(p0.ravel())
  for c in cams:
    init_poses.append(list(c['poses'][0][0].ravel()))
    init_poses[-1].extend(c['poses'][0][1].ravel())
    for p2d in c['pts2d']:
      pts2d.extend(p2d.ravel())

  for i in range(len(cams[0]['img_files'])):

    if i == 0:

      continue

    r0 = cams[0]['poses'][0][0]

    t0 = cams[0]['poses'][0][1]

    r = cams[0]['poses'][i][0]

    t = cams[0]['poses'][i][1]
    [rt, tt] = relative_transform(r0, t0, r, t)
    for p0 in pts3d:

      pt = rt @ p0 + np.transpose(tt)
      init_pts3d.extend(pt.ravel())

  return [init_poses, init_pts3d, pts2d]

