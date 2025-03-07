# UE5 Project Setup 

This assumes you have Unreal 5.3, Visual Studio 2022, and git with `git-bash` to run the `init.sh` and `clean.sh`. For telepresence, you should have steam installed, running, and logged in, before you can use any of the launch scripts.

2. Create a new UE5 project
	1. You can use either high quality or scalable, with the latter trading some visual quality for performance.
3. Close the UE5 editor
4. Open `git-bash` in the root folder and create a repository
	1. `git init`
5. Add the VIMR instance template folder as a sub-repo
	1. `git submodule add --depth 1 https://git.sjmd.dev/stu/VIMRUE5_instance_template.git ./VIMR`
6. Get the other plugins as sub-repos, and init some scripts and things
	1. `cd VIMR; ./setup.sh; ./sync-init.sh`
7. Modify `DefaultEngine.ini` and `DefaultGame.ini` as described in the VIMRU5 plugin Readme
8. Run `VIMR/lib/set_vimr_root.bat`
9. Run `clean.sh` - you can then open the UE5 project, and click 'yes' when asked if you want to rebuild the plugins.
10. Once it's open, enable some plugins and change some project settings:
	1. Enable the OnlineSubsystemSteam plugin
	2. Change the GameInstance to TelepresenceInstance
	3. Change the Startup Map to WaitingMap
11. Edit `VIMR/vimr_env.bat` to correctly point to your UE5 installation
12. You can now use `editor-stream.bat` to launch a standalone version of the game, as well as the default camera capture configured in `VIMR/instance`.
13. For telepresence:
	1. Edit `VIMR/vimr_env_common.bat`
	2. Share the root folder with other computer(s) using syncthing. Wait for the computers to sync.
	3. On each synced computer, Run `VIMR/sync-init.sh`, then r `VIMR/lib/set_vimr_root.bat`, then `clean.sh`
	4. Also make sure the UE5 path is set correctly in `VIMR/vimr_env.bat` for each computer
	5. Open the UE5 project, and click 'yes' to rebuild plugins 
	6. You don't need to do Step 7 or step 10 on the synced computers
	7. Make sure steam is running
	8. Run `editor-create-session.bat` on one computer to start a telepresence session
	9. Run `editor-join-first-session.bat` on each other computer to join that session