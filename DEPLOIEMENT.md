# Déploiement de Smart Weather Management

## Erreur « Qt6Sql.dll est introuvable »

Cette erreur apparaît lorsque `smartweather.exe` est lancé seul, sans les DLL Qt
à côté de lui. Le compilateur et Qt Creator fonctionnent car Qt est alors dans le
`PATH`, mais un double-clic sur l'exécutable ne bénéficie pas de ce `PATH`.

Le dossier `release` a été déployé avec `windeployqt` et contient maintenant :

- `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll`, `Qt6Sql.dll` ;
- les bibliothèques d'exécution MinGW (`libgcc_s_seh-1.dll`, `libstdc++-6.dll`,
  `libwinpthread-1.dll`) ;
- les plugins `platforms\qwindows.dll`, `sqldrivers\qsqlite.dll`,
  `sqldrivers\qsqlodbc.dll`, etc.

**Lancement :** double-cliquer sur `release\smartweather.exe`.
**Important :** conserver tout le dossier `release`, pas seulement l'exécutable.

## Recompiler puis redéployer

```bat
qmake smartweather.pro
mingw32-make -j8
deployer.bat
```

`deployer.bat` utilise `windeployqt` (chemin Qt par défaut :
`C:\Qt\6.7.3\mingw_64\bin`, ou `QTDIR`, ou le `PATH`).

## Base de données

La base SQLite utilisée par l'application se trouve dans :

```text
%APPDATA%\SWM\SmartWeatherManagement\smartweather.db
```

Une sauvegarde a été créée dans :

```text
%APPDATA%\SWM\SmartWeatherManagement\sauvegardes\
```

L'application tente Oracle en premier, puis bascule sur SQLite locale si Oracle
n'est pas disponible. Pour Oracle, appliquer `sql/oracle_migration.sql` et
notamment élargir `EMPLOYES.TELEPHONE` à `VARCHAR2(25)`.
