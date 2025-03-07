
1. You can delete `Plugins/AdvancedSteamSessions`
2. Edit `config/DefaultEngine.ini` and `config/DefaultGame.ini`

**`DefaultEngine.ini`**

```
[Voice] 
bEnabled=true

[OnlineSubsystem]
bHasVoiceEnabled=true

[SystemSettings]
voice.SilenceDetectionThreshold=0.01
voice.MicNoiseGateThreshold=0.01
```

**`DefaultGame.ini`**

```
[/script/Engine.GameSession]
bRequiresPushToTalk=false
```
