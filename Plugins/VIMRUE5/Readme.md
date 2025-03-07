# VIMRUE5_Plugin

VoxelVideo streaming and playback for UnrealEngine 5.3.

## Dependencies

- [3DConnexion SpaceMouse Driver and Unreal Plugin](https://3dconnexion.com/uk/drivers/)
- https://github.com/microdee/UE4-SpaceMouse
- https://github.com/mordentral/AdvancedSessionsPlugin
  - AdvancedSessions is required
  - AdvancedSteamSession is only required if using Steam

## Configuration for LAN vs Steam

See [Readme-LAN](./Readme-LAN.md) for configuring the project for telepresence over LAN connections only (this is the default).

See [Readme-Steam](./Readme-Steam.md) for configuring the project for telepresence over the internet using Steam.

## Performance and Voxel Count

To reduce the number of voxels (e.g. for performance on less capable hardware) `MAX_RENDERER_VOXELS` defined in `VoxelRenderComponent.h`.
To increase voxels above the hard limit of 196608 voxels per renderer, edit the `NumVoxels`. Do this if there are blocks of voxels missing when rendered.

## Citation

```bibtex
@article{duncan2021voxelbased,
    author = {Duncan, Stuart and Park, Noel and Ott, Claudia and Langlotz, Tobias and Regenbrecht, Holger Regenbrecht},
    title = {Voxel-Based Immersive Mixed Reality: A Framework for Ad Hoc Immersive Storytelling},
    journal = {PRESENCE: Virtual and Augmented Reality},
    volume = {30},
    pages = {5-29},
    year = {2021},
    month = {12},
    issn = {1054-7460},
    doi = {10.1162/pres_a_00364},
}
```