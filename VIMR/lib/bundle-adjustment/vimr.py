import numpy as np
import cv2 as cv
import json
import os
import utils
def vimr_gen_checkerboard_3dpts(dims, square_edge, offset=[0, 0, 0], swap_xz=False):
    """
    Generate a set of 3D points which coincide with checkerboard corners.

    for the inner corners of a checkerboard aligned with the Y-Z plane, and
    with a given offset from the world origin.

    Parameters
    ----------
    dims : [x,z]
      Number of inner corners of the checkerboard in the Y and Z directions

    square_edge : n
      Length of a square edge in real world units, this defines the units of
      the translation part of the transform estimated by SolvePnP

    offset : [x,y,z]
      Offset from the world origin to the center of the checkerboard, in the
      same units as square_edge

    horizontal : bool
      Whether the board is oriented vertically or horizontally
    """
    pts = []
    w = dims[0]
    h = dims[1]
    for y in range(h):
        for x in range(w):
          X = (x - w/2 + 0.5) * square_edge
          Y = (y - h/2 + 0.5) * square_edge
          if swap_xz: 
            pts.append([-X, Y, 0])
          else:
            pts.append([0, -X, Y])
    return np.array(pts)

def vimr_init(instance_path):
  with open(instance_path + '/instance.json') as json_file:
    vconf = json.loads(json_file.read())

  vc = vconf['Components']
  cams =[]
  for c in vc:
    if 'CamType' in vc[c]:
      cams.append({
        'id': c,
        'path': os.path.join(instance_path, c),
        'img_files':[],
        'pts2d':[],
        'poses':[]
      })

  cbc = vconf['Checkerboard']
  board_dims = [cbc['X'], cbc['Y']]

  board = vimr_gen_checkerboard_3dpts(board_dims, cbc['SquareEdge_m'], cbc['TransFromOrigin_m'], cbc['SwapXZ'])

  for cam in cams:
    itr_path = os.path.join(cam['path'], cam['id'] + ".intrinsic.yaml")
    if os.path.exists(itr_path):
      ifs = cv.FileStorage(itr_path, cv.FileStorage_READ)
      cam['intrinsics'] = ifs.getNode("intrinsic-ir0").mat()
      if cam['intrinsics'] is None:
        raise IOError("Intrinsics file does not contain 'intrinsic-ir0' key")
    else:
      raise IOError("Cam intrinsic file missing: {}".format(itr_path))
    for tf in os.listdir(cam['path']):
      if(tf.endswith('.yaml')):
        print("intrinsics file: {}".format(tf))
    
    for tf in os.listdir(cam['path']):
      if(tf.endswith(".png")):
        img_path = os.path.join(cam['path'], tf)
        [found, pts2d, r, t] = utils.find_board(img_path, board_dims, cam['intrinsics'], board)
        if not found:
          raise IOError('failed to find checkerboard in image {}'.format(img_path))
        cam['img_files'].append(img_path)
        cam['pts2d'].append(pts2d.ravel())
        cam['poses'].append([r,t])

    if 'intrinsics' not in cam:
      raise IOError('Intrinsics missing for {}'.format(cam['id']))
  return [cams, board_dims, board]