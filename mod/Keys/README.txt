TEST BUILD - UNSIGNED

This PBO is not signed yet. For testing either:
  - set  verifySignatures = 0;  in serverDZ.cfg, or
  - sign it yourself with DayZ Tools (Windows):
      DSCreateKey.exe HOD
      DSSignFile.exe HOD.biprivatekey @HOD_AdminTools\Addons\HOD_AdminTools.pbo
    then put HOD.bikey in this Keys folder and in the server's keys folder.
Keep HOD.biprivatekey private - never ship it.
