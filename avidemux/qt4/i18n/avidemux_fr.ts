<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr">
<context>
    <name>ADMImage</name>
    <message>
        <source>Memory error</source>
        <translation type="obsolete">Impossible d&apos;allouer la mémoire</translation>
    </message>
    <message>
        <source>Something bad happened</source>
        <translation type="obsolete">Quelque chose d&apos;anormal c&apos;est produit</translation>
    </message>
</context>
<context>
    <name>ADMVideoDenoise</name>
    <message>
        <source>L_uma threshold:</source>
        <translation type="obsolete">Seuil de Luma:</translation>
    </message>
    <message>
        <source>Ch_roma threshold:</source>
        <translation type="obsolete">Seuil de Chroma:</translation>
    </message>
    <message>
        <source>_Scene change:</source>
        <translation type="obsolete">Changement de scène:</translation>
    </message>
    <message>
        <source>Denoise</source>
        <translation type="obsolete">Débruitage</translation>
    </message>
</context>
<context>
    <name>ADMVideoDropOut</name>
    <message>
        <source>DropOut Threshold</source>
        <translation type="obsolete">Seuil d&apos;élimination</translation>
    </message>
    <message>
        <source>Drop Out</source>
        <translation type="obsolete">Elimination</translation>
    </message>
</context>
<context>
    <name>ADMVideoFlux</name>
    <message>
        <source>_Temporal threshold:</source>
        <translation type="obsolete">Seuil temporel:</translation>
    </message>
    <message>
        <source>_Spatial threshold:</source>
        <translation type="obsolete">Seuil spacial:</translation>
    </message>
</context>
<context>
    <name>ADMVideoForcedPP</name>
    <message>
        <source>_Filter strength:</source>
        <translation type="obsolete">Force du filtre:</translation>
    </message>
    <message>
        <source>_Horizontal deblocking</source>
        <translation type="obsolete">Deblocking horizontal</translation>
    </message>
    <message>
        <source>_Vertical deblocking</source>
        <translation type="obsolete">Deblocking vertical</translation>
    </message>
    <message>
        <source>Forced Postprocessing</source>
        <translation type="obsolete">Postprocessing forcé</translation>
    </message>
</context>
<context>
    <name>ADMVideoMPD3D</name>
    <message>
        <source>_Spatial luma strength:</source>
        <translation type="obsolete">Luma, intensité spatiale:</translation>
    </message>
    <message>
        <source>S_patial chroma strength:</source>
        <translation type="obsolete">Chorma, intensité spatiale:</translation>
    </message>
    <message>
        <source>Luma _Temporal strength:</source>
        <translation type="obsolete">Luma, intensité temporelle:</translation>
    </message>
</context>
<context>
    <name>ADMVideoMPD3Dlow</name>
    <message>
        <source>_Spatial luma strength:</source>
        <translation type="obsolete">Luma, intensité spatiale:</translation>
    </message>
    <message>
        <source>S_patial chroma strength:</source>
        <translation type="obsolete">Chroma, intensité spatiale:</translation>
    </message>
    <message>
        <source>_Temporal strength:</source>
        <translation type="obsolete">Intensité temporelle:</translation>
    </message>
</context>
<context>
    <name>ADMVideoMaskedSoften</name>
    <message>
        <source>_Luma threshold:</source>
        <translation type="obsolete">Seuil de _luma:</translation>
    </message>
    <message>
        <source>C_hroma threshold:</source>
        <translation type="obsolete">Seuil de c_hroma:</translation>
    </message>
    <message>
        <source>_Radius:</source>
        <translation type="obsolete">_Rayon:</translation>
    </message>
</context>
<context>
    <name>ADMVideoMosaic</name>
    <message>
        <source>_Horizontal stacking:</source>
        <translation type="obsolete">Empilement horitzontal:</translation>
    </message>
    <message>
        <source>_Vertical stacking:</source>
        <translation type="obsolete">Empilement vertical:</translation>
    </message>
    <message>
        <source>_Shrink factor:</source>
        <translation type="obsolete">Miniaturisation:</translation>
    </message>
    <message>
        <source>Show _frame</source>
        <translation type="obsolete">Montrer les images</translation>
    </message>
</context>
<context>
    <name>ADM_Composer</name>
    <message>
        <location filename="../../common/ADM_editor/src/utils/ADM_edCheckForInvalidPts.cpp" line="+69"/>
        <source>Some timing information are incorrect.
It happens with some capture software.
If you re encode video we should drop these informations,
 else it will cause dropped frame/jerky video.
If you just copy the video without reencoding,
 you should keep them.
Drop timing informations ?</source>
        <translation>Les informations de timings sur les images sont incorrectes.
Cela arrive avec certains logiciels de capture.
Si vous re-encoder ces videos, il vaut mieux ignorer ces informations,
sinon cela va provoquer des saccades.
Si vous copiez la piste sans re-encoder, il vaut mieux conserver ces informations de timing.
Ignorer les informations de timing ?</translation>
    </message>
    <message>
        <location filename="../../common/ADM_editor/src/ADM_edit.cpp" line="+178"/>
        <source>Cannot find a demuxer for %s</source>
        <translation>Impossible de trouver un demuxer pour lire %s</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Attempt to open %s failed!</source>
        <translation>l&apos;ouverture de %s a echouée !</translation>
    </message>
    <message>
        <source>You cannot mix different video dimensions yet. Using the partial video filter later, will not work around this problem. The workaround is:
1.) &quot;resize&quot; / &quot;add border&quot; / &quot;crop&quot; each stream to the same resolution
2.) concatinate them together</source>
        <translation type="vanished">Vous ne pouvez pas mélanger des videos avec des dimensions différentes. Pour ce faire, editer les vidéos avec les filtres resize/etc.. afin qu&apos;elles aient toutes la même taille, puis les ajouter</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>You cannot mix different video dimensions yet. Using the partial video filter later, will not work around this problem. The workaround is:
1.) &quot;resize&quot; / &quot;add border&quot; / &quot;crop&quot; each stream to the same resolution
2.) concatenate them together</source>
        <translation>Vous ne pouvez pas mélanger des videos avec dimensions différentes. Utiliser le filtre &quot;Partiel&quot; ne résoudra pas le problème.La correction est :
1) Redimensionner/ajouter des bords pour que toutes les vidéos aient les mêmes dimensions, et les sauver une par une
2) Concaténer ces videos ensembles</translation>
    </message>
    <message>
        <location filename="../../common/ADM_editor/src/utils/ADM_edCheckForInvalidPts.cpp" line="-18"/>
        <source>Checking if timestamps are valid..</source>
        <translation>Vérification des informations de temps</translation>
    </message>
</context>
<context>
    <name>ADM_Qt4CoreUIToolkit</name>
    <message>
        <source>Alert</source>
        <translation type="vanished">Alerte</translation>
    </message>
    <message>
        <source>Info</source>
        <translation type="vanished">Information</translation>
    </message>
</context>
<context>
    <name>ADM_Qt4CoreUIToolkit::DIA_processingQt4</name>
    <message>
        <source>_Resume</source>
        <translation type="obsolete">_Reprendre</translation>
    </message>
    <message>
        <source>The processing is paused.</source>
        <translation type="obsolete">L&apos;operation est en pause</translation>
    </message>
    <message>
        <source>Cancel it ?</source>
        <translation type="obsolete">Annuler ?</translation>
    </message>
</context>
<context>
    <name>ADM_Qt4Factory::ADM_Qbitrate</name>
    <message>
        <source>Constant Bitrate</source>
        <translation type="vanished">Bitrate constant</translation>
    </message>
    <message>
        <source>Constant Quantiser</source>
        <translation type="vanished">Quantisation constante</translation>
    </message>
    <message>
        <source>Same Quantiser as Input</source>
        <translation type="vanished">Même quantisation que la source</translation>
    </message>
    <message>
        <source>Constant Rate Factor</source>
        <translation type="vanished">Rate Factor constant</translation>
    </message>
    <message>
        <source>Two Pass - Video Size</source>
        <translation type="vanished">2 Passes- Taille de la vidéo</translation>
    </message>
    <message>
        <source>Two Pass - Average Bitrate</source>
        <translation type="vanished">2 Passes - Bitrate moyen</translation>
    </message>
    <message>
        <source>Encoding mode</source>
        <translation type="vanished">Mode d&apos;encodage</translation>
    </message>
    <message>
        <source>Target bitrate (kb/s)</source>
        <translation type="vanished">Bitrate cible (kb/s)</translation>
    </message>
    <message>
        <source>Quantizer</source>
        <translation type="vanished">Quantisation</translation>
    </message>
    <message>
        <source>Target video size (MB)</source>
        <translation type="vanished">Taille cible de la vidéo (MB)</translation>
    </message>
    <message>
        <source>Average bitrate (kb/s)</source>
        <translation type="vanished">Bitrate moyen (kb/s)</translation>
    </message>
</context>
<context>
    <name>ADM_QthreadCount</name>
    <message>
        <location filename="../ADM_UIs/src/T_threadCount.cpp.rej" line="+7"/>
        <source>Disabled</source>
        <translation>Désactivé</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Auto-detect</source>
        <translation>Détection auto</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Custom</source>
        <translation>Manuel</translation>
    </message>
</context>
<context>
    <name>ADM_qt4Factory::ADM_QthreadCount</name>
    <message>
        <location filename="../ADM_UIs/src/T_threadCount.cpp.orig" line="+45"/>
        <source>Disabled</source>
        <translation>Désactivé</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Auto-detect</source>
        <translation>Détection auto</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Custom</source>
        <translation>Manuel</translation>
    </message>
</context>
<context>
    <name>ADM_qtray</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/ADM_qtray.cpp.orig" line="+82"/>
        <location filename="../ADM_userInterfaces/ADM_gui/ADM_qtray.cpp.rej" line="+7"/>
        <source>Open Avidemux</source>
        <translation>Ouvrir Avidemux</translation>
    </message>
</context>
<context>
    <name>AVDM_Fade</name>
    <message>
        <source>_Start time (ms):</source>
        <translation type="obsolete">Début (ms):</translation>
    </message>
    <message>
        <source>_End time (ms):</source>
        <translation type="obsolete">Fin(ms):</translation>
    </message>
    <message>
        <source>Fade to black</source>
        <translation type="obsolete">Fondu au noir</translation>
    </message>
</context>
<context>
    <name>CalculatorDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/calculator.ui" line="+129"/>
        <source>Custom</source>
        <translation>Manuel</translation>
    </message>
    <message>
        <location line="+143"/>
        <source>Track 1:</source>
        <translation>Piste 1:</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Track 2:</source>
        <translation>Piste 2:</translation>
    </message>
    <message>
        <location line="+330"/>
        <source>Cancel</source>
        <translation>Annuler</translation>
    </message>
    <message>
        <location line="-623"/>
        <source>Calculator</source>
        <translation></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Target</source>
        <translation>Cible</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>00:00:00</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>AVI</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>OGM</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>MPEG</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Medium:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Format:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Duration:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>1 x 80 minute CD</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2 x 80 minute CD</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>1 x 74 minute CD</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2 x 74 minute CD</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>DVD-5</source>
        <translation></translation>
    </message>
    <message>
        <location line="+63"/>
        <source>Custom Size:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+23"/>
        <location line="+198"/>
        <location line="+27"/>
        <location line="+34"/>
        <source>MB</source>
        <translation></translation>
    </message>
    <message>
        <location line="-217"/>
        <source>Audio Bitrate</source>
        <translation></translation>
    </message>
    <message>
        <location line="+34"/>
        <location line="+37"/>
        <location line="+232"/>
        <source>kbps</source>
        <translation></translation>
    </message>
    <message>
        <location line="-207"/>
        <source>Result</source>
        <translation></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Audio Size:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Video Size:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+27"/>
        <location line="+34"/>
        <location line="+68"/>
        <location line="+18"/>
        <source>0</source>
        <translation></translation>
    </message>
    <message>
        <location line="-101"/>
        <source>Total Size:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+120"/>
        <source>Bits Per Pixel:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Video Bitrate:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+61"/>
        <source>OK</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>DGbob</name>
    <message>
        <source>Keep nb of frames and fps</source>
        <translation type="obsolete">Garder les images et le nombre d&apos;i/s</translation>
    </message>
    <message>
        <source>Double nb of frames and fps</source>
        <translation type="obsolete">Doubler les images et le nombre d&apos;i/s</translation>
    </message>
    <message>
        <source>Double nb of frames (slow motion)</source>
        <translation type="obsolete">Doubler le nombre d&apos;images (ralenti)</translation>
    </message>
    <message>
        <source>_Threshold:</source>
        <translation type="obsolete">_Seuil:</translation>
    </message>
</context>
<context>
    <name>DIA_encodingQt4</name>
    <message>
        <source>Shutting down</source>
        <translation type="vanished">Extinction en cours</translation>
    </message>
    <message>
        <source>The encoding is paused. Do you want to resume or abort?</source>
        <translation type="vanished">L&apos;encodage est en pause. Voulez vous reprendre ou abandonner ?</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation type="vanished">Reprendre</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="vanished">Abandonner</translation>
    </message>
</context>
<context>
    <name>DIA_progressIndexing</name>
    <message>
        <source>Time Left :%02d:%02d:%02d</source>
        <translation type="obsolete">Temps restant :%02d:%02d:%02d</translation>
    </message>
</context>
<context>
    <name>Decimate</name>
    <message>
        <source>Discard closer</source>
        <translation type="obsolete">Eliminer le plus proche</translation>
    </message>
    <message>
        <source>Replace (interpolate)</source>
        <translation type="obsolete">Remplacer (interpoler)</translation>
    </message>
    <message>
        <source>Discard longer dupe (animÃ©s)</source>
        <translation type="obsolete">Eliminer la séquence répétée la plus longue (animés)</translation>
    </message>
    <message>
        <source>Pulldown dupe removal</source>
        <translation type="obsolete">Elimination du pulldown</translation>
    </message>
    <message>
        <source>Fastest (no chroma, partial luma)</source>
        <translation type="obsolete">Rapide (luma partielle)</translation>
    </message>
    <message>
        <source>Medium (full luma, no chroma)</source>
        <translation type="obsolete">Moyen (toute la luma)</translation>
    </message>
    <message>
        <source>_Quality:</source>
        <translation type="obsolete">_Qualité:</translation>
    </message>
    <message>
        <source>_Threshold 1:</source>
        <translation type="obsolete">_Seuil 1:</translation>
    </message>
    <message>
        <source>T_hreshold 2:</source>
        <translation type="obsolete">S_euil2:</translation>
    </message>
    <message>
        <source>Sho_w</source>
        <translation type="obsolete">A_fficher</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/T_index_pg.cpp" line="+64"/>
        <source>Indexing</source>
        <translation>Indexation</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Time Left : Infinity</source>
        <translation>Temps Restant : Infini</translation>
    </message>
    <message>
        <location line="+1"/>
        <source># Images :</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>DialogAudioTracks</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/audioTracks.ui" line="+14"/>
        <source>Audio Tracks Configuration</source>
        <translation>Configuration des pistes audios</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Track 1</source>
        <translation>Piste 1</translation>
    </message>
    <message>
        <location line="+7"/>
        <location line="+41"/>
        <location line="+41"/>
        <location line="+41"/>
        <source>Enabled</source>
        <translation>Activée</translation>
    </message>
    <message>
        <location line="-107"/>
        <location line="+41"/>
        <location line="+41"/>
        <location line="+41"/>
        <source>Configure</source>
        <translation>Configurer</translation>
    </message>
    <message>
        <location line="-116"/>
        <location line="+41"/>
        <location line="+41"/>
        <location line="+41"/>
        <source>Filters</source>
        <translation>Filtres</translation>
    </message>
    <message>
        <source>Audio Filters</source>
        <translation type="vanished">Filtres</translation>
    </message>
    <message>
        <location line="-112"/>
        <source>Track 2</source>
        <translation>Piste 2</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Track 3</source>
        <translation>Piste 3</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Track 4</source>
        <translation>Piste 4</translation>
    </message>
</context>
<context>
    <name>DialogOcr</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_ocr/ocr.ui" line="+92"/>
        <source>Close</source>
        <translation>Fermer</translation>
    </message>
    <message>
        <location line="-79"/>
        <source>Dialog</source>
        <translation></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Calibrate</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Skip All</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Skip Glyph</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Ignore</source>
        <translation></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Ok</source>
        <translation></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>00:00:00/000</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Timecode:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>0/0</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Nb Lines :</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>DialogProcessing</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/processing.ui" line="+33"/>
        <source>Cancel</source>
        <translation>Annuler</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Dialog</source>
        <translation></translation>
    </message>
    <message>
        <location line="+26"/>
        <source>TimeLeft</source>
        <translation>Temps restant</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>00:00:00</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>ProcessImages</source>
        <translation>Images Traitées</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>0</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Speed</source>
        <translation>Vitesse</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>0 fps</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>DialogProgress</name>
    <message>
        <location filename="../ADM_jobs/src/uiProgress.ui" line="+17"/>
        <source>Progress</source>
        <translation>Progression</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Current job</source>
        <translation>Job en cours</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>File being written </source>
        <translation>Ecriture fichier</translation>
    </message>
    <message>
        <location line="-7"/>
        <source>1/1</source>
        <translation></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>...</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>GUIPlayback</name>
    <message>
        <source>Trouble initializing audio device</source>
        <translation type="vanished">Le device audio ne peut pas être initialisé</translation>
    </message>
</context>
<context>
    <name>MP4Header</name>
    <message>
        <source>Problem reading SVQ3 headers</source>
        <translation type="vanished">Impossible de lire l&apos;entete SVQ3</translation>
    </message>
    <message>
        <source>No stts table</source>
        <translation type="vanished">Pas d&apos;element STTS</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/gui2.ui" line="+15"/>
        <source>Avidemux</source>
        <translation>Avidemux</translation>
    </message>
    <message>
        <location line="+102"/>
        <source>&amp;Help</source>
        <translation>&amp;Aide</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Edit</source>
        <translation>&amp;Editer</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;View</source>
        <translation>&amp;Vue</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>&amp;Tools</source>
        <translation>Ou&amp;tils</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;File</source>
        <translation>&amp;Fichier</translation>
    </message>
    <message>
        <source>Codec Options</source>
        <translation type="obsolete">Options</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>&lt;b&gt;Video Decoder&lt;b&gt;</source>
        <translation>&lt;b&gt;Décodeur Vidéo&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+100"/>
        <location line="+142"/>
        <location line="+169"/>
        <location line="+181"/>
        <source>Configure</source>
        <translation>Configurer</translation>
    </message>
    <message>
        <location line="-453"/>
        <source>&lt;b&gt;Video Output&lt;/b&gt;</source>
        <translation>&lt;b&gt;Sortie Vidéo&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+86"/>
        <location line="+169"/>
        <source>Copy</source>
        <translation>Copier</translation>
    </message>
    <message>
        <location line="-136"/>
        <location line="+169"/>
        <source>Filters</source>
        <translation>Filtres</translation>
    </message>
    <message>
        <location line="-112"/>
        <source>&lt;b&gt;Audio Output&lt;/b&gt;</source>
        <translation>&lt;b&gt;Sortie Audio&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>   (0 track(s))</source>
        <translation>   (0 piste(s))</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>&amp;Shift:</source>
        <translation>&amp;Décal.:</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>&lt;b&gt;Output Format&lt;/b&gt;</source>
        <translation>&lt;b&gt;Format de sortie&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+968"/>
        <source>&amp;Open...</source>
        <translation>&amp;Ouvrir...</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Save &amp;Video...</source>
        <translation>Sauver la &amp;Vidéo...</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+3"/>
        <source>Save Video</source>
        <translation>Sauver</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Load/Run Project...</source>
        <translation>&amp;Lancer un projet...</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+3"/>
        <source>Load/Run Project</source>
        <translation>Lancer un projet</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;Project</source>
        <translation>Sauver un &amp;Projet</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+3"/>
        <source>Save Project</source>
        <translation>Sauver un Projet</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Calculator...</source>
        <translation>&amp;Calculette...</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>&amp;Input</source>
        <translation>Orig&amp;inal</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Output</source>
        <translation>M&amp;odifié</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Codec Options</source>
        <translation>Options du co&amp;dec</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>&amp;Separate</source>
        <translation>&amp;Séparer</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Top</source>
        <translation>Hau&amp;t</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Si&amp;de</source>
        <translation>Cot&amp;e à cote</translation>
    </message>
    <message>
        <location line="-1824"/>
        <source>Vi&amp;deo</source>
        <translation></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&amp;Toolbars</source>
        <translation></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Auto</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Custom</source>
        <translation>&amp;Scripts Perso</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Go</source>
        <translation>Aller</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Audio</source>
        <translation></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Recent</source>
        <translation>Récent</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Codec O&amp;ptions</source>
        <translation></translation>
    </message>
    <message>
        <location line="+108"/>
        <location line="+7"/>
        <source>XXXX</source>
        <translation></translation>
    </message>
    <message>
        <location line="+404"/>
        <source>ms</source>
        <translation></translation>
    </message>
    <message>
        <location line="+149"/>
        <location line="+978"/>
        <source>&amp;Navigation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-903"/>
        <source>Play [SPACE]</source>
        <translation>Lire [SPACE]</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Play</source>
        <translation>Lire</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+26"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+23"/>
        <location line="+26"/>
        <location line="+352"/>
        <source>...</source>
        <translation></translation>
    </message>
    <message>
        <location line="-637"/>
        <source>Stop [SPACE]</source>
        <translation></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to previous frame [LEFT]</source>
        <translation>Image précédents [GAUCHE]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to next frame [RIGHT]</source>
        <translation>Image suivante [DROITE]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to previous keyframe [DOWN]</source>
        <translation>Image clé précédente [BAS]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to next keyframe [UP]</source>
        <translation>Image clé suivante [HAUT]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Set start marker [CTRL+PAGEUP]</source>
        <translation>Mettre le marqueur A [CTRL+PAGE HAUTE]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Set end marker [CTRL+PAGEDOWN]</source>
        <translation>Mettre le marqueur B [CTRL+PAGE SUIVANTE]</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Search previous black frame</source>
        <translation>Chercher l&apos;image noire précédente</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Search next black frame</source>
        <translation>Chercher l&apos;image noire suivante</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to first frame [HOME]</source>
        <translation>Aller au début [DEBUT]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Go to last frame [END]</source>
        <translation>Aller à la fin [FIN]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Backward one minute [CTRL+DOWN]</source>
        <translation>Reculer d&quot;une minute [CTRL+BAS]</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Forward one minute [CTRL+UP]</source>
        <translation>Avancer d&apos;une minute [CTRL+HAUT]</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Forward one minute</source>
        <translation>Avancer d&apos;une minute</translation>
    </message>
    <message>
        <location line="+58"/>
        <source>Time:</source>
        <translation>Temps:</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>00:00:00.000</source>
        <translation></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>/ 00:00:00.000</source>
        <translation></translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Frame type:</source>
        <translation>Type:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>?</source>
        <translation></translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Se&amp;lection</source>
        <translation></translation>
    </message>
    <message>
        <location line="+73"/>
        <source>A:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>B:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Go to marker A [PAGE UP]</source>
        <translation>Aller au marqueur A [PAGE HAUT]</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+10"/>
        <source>000000</source>
        <translation></translation>
    </message>
    <message>
        <location line="-3"/>
        <source>Go to Marker B [PAGE DOWN]</source>
        <translation>Aller au marqueur B [PAGE BAS]</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Display output video on playback</source>
        <translation>Jouer la video post filtrage</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Play filtered</source>
        <translation>Jouer post filtrage</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Vol&amp;ume</source>
        <translation></translation>
    </message>
    <message>
        <location line="+100"/>
        <source>Audio &amp;Metre</source>
        <translation></translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Open Video</source>
        <translation>Ouvrir une vidéo</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Information...</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Alt+Return</source>
        <translation></translation>
    </message>
    <message>
        <location line="+57"/>
        <source>F7</source>
        <translation></translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Ctrl+Alt+C</source>
        <translation></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Ctrl+Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Selection</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Alt+S</source>
        <translation></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Volume</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Alt+V</source>
        <translation></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Audio Metre</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Alt+M</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Msharpen</name>
    <message>
        <source>_Strength:</source>
        <translation type="obsolete">Force:</translation>
    </message>
</context>
<context>
    <name>Msmooth</name>
    <message>
        <source>_Strength:</source>
        <translation type="obsolete">Force:</translation>
    </message>
</context>
<context>
    <name>OpenDMLHeader</name>
    <message>
        <source>Malformed header</source>
        <translation type="vanished">En tete incorrecte</translation>
    </message>
    <message>
        <source>Unpacking bitstream</source>
        <translation type="vanished">Conversion en cours</translation>
    </message>
</context>
<context>
    <name>SpiderMonkeyShell</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_shell/shell.ui" line="+17"/>
        <source>Shell</source>
        <translation></translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Clear</source>
        <translation>Effacer</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Evaluate</source>
        <translation>Evaluer</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Return</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Telecide</name>
    <message>
        <source>None</source>
        <translation type="obsolete">Aucun</translation>
    </message>
</context>
<context>
    <name>Ui_licenseWindow</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_license.cpp.orig" line="+25"/>
        <source>&lt;!DOCTYPE html PUBLIC &quot;-//W3C//DTD HTML 4.01 Transitional//EN&quot;&gt;&lt;html&gt;&lt;head&gt;  &lt;title&gt;Avidemux is free software; you can redistribute it and/or  modify it under the terms of the GNU General Public License  version 2 as published by the Free Software Foundation&lt;/title&gt;&lt;style type=&quot;text/css&quot;&gt;&lt;!-- /* Style Definitions */ p.licenseStyle, li.licenseStyle, div.licenseStyle        {margin:0cm;        margin-bottom:.0001pt;        font-size:12.0pt;        font-family:&quot;Times New Roman&quot;;} /* Page Definitions */ @page Section1        {size:612.0pt 792.0pt;        margin:72.0pt 90.0pt 72.0pt 90.0pt;}div.Section1        {page:Section1;}--&gt;&lt;/style&gt;&lt;/head&gt;&lt;body lang=&quot;EN-GB&quot; style=&apos;text-justify-trim:punctuation&apos;&gt;  &lt;div class=&quot;Section1&quot;&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt;Avidemux is    free software; you can redistribute it and/or modify it under    the terms of the GNU General Public License version 2 as    published by the Free Software Foundation.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;b&gt;&lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt;    &lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;GNU GENERAL PUBLIC    LICENSE&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Version 2, June    1991&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Copyright (C) 1989, 1991    Free Software Foundation, Inc.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span lang=&quot;FR&quot; style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;59 Temple Place, Suite    330, Boston, MA  02111-1307  USA&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span lang=&quot;FR&quot; style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Everyone is permitted to    copy and distribute verbatim copies of this license document,    but changing it is not allowed.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Preamble&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The licenses for most    software are designed to take away your freedom to share and    change it.  By contrast, the GNU General Public License is    intended to guarantee your freedom to share and change free    software--to make sure the software is free for all its users.     This General Public License applies to most of the Free    Software Foundation&apos;s software and to any other program whose    authors commit to using it.  (Some other Free Software    Foundation software is covered by the GNU Library General    Public License instead.)  You can apply it to your programs,    too.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;When we speak of free    software, we are referring to freedom, not price.  Our General    Public Licenses are designed to make sure that you have the    freedom to distribute copies of free software (and charge for    this service if you wish), that you receive source code or can    get it if you want it, that you can change the software or use    pieces of it in new free programs; and that you know you can do    these things.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;To protect your rights, we    need to make restrictions that forbid anyone to deny you these    rights or to ask you to surrender the rights. These    restrictions translate to certain responsibilities for you if    you distribute copies of the software, or if you modify    it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;For example, if you    distribute copies of such a program, whether gratis or for a    fee, you must give the recipients all the rights that you have.     You must make sure that they, too, receive or can get the    source code.  And you must show them these terms so they know    their rights.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;We protect your rights    with two steps: (1) copyright the software, and (2) offer you    this license which gives you legal permission to copy,    distribute and/or modify the software.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Also, for each author&apos;s    protection and ours, we want to make certain that everyone    understands that there is no warranty for this free software.     If the software is modified by someone else and passed on, we    want its recipients to know that what they have is not the    original, so that any problems introduced by others will not    reflect on the original authors&apos; reputations.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Finally, any free program    is threatened constantly by software patents.  We wish to avoid    the danger that redistributors of a free program will    individually obtain patent licenses, in effect making the    program proprietary.  To prevent this, we have made it clear    that any patent must be licensed for everyone&apos;s free use or not    licensed at all.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The precise terms and    conditions for copying, distribution and modification    follow.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;GNU GENERAL PUBLIC    LICENSE&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;TERMS AND CONDITIONS FOR    COPYING, DISTRIBUTION AND MODIFICATION&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;0. This License applies to    any program or other work which contains a notice placed by the    copyright holder saying it may be distributed under the terms    of this General Public License.  The &quot;Program&quot;, below, refers    to any such program or work, and a &quot;work based on the Program&quot;    means either the Program or any derivative work under copyright    law: that is to say, a work containing the Program or a portion    of it, either verbatim or with modifications and/or translated    into another language.  (Hereinafter, translation is included    without limitation in the term &quot;modification&quot;.)  Each licensee    is addressed as &quot;you&quot;.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Activities other than    copying, distribution and modification are not covered by this    License; they are outside its scope.  The act of running the    Program is not restricted, and the output from the Program is    covered only if its contents constitute a work based on the    Program (independent of having been made by running the    Program). Whether that is true depends on what the Program    does.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;1. You may copy and    distribute verbatim copies of the Program&apos;s source code as you    receive it, in any medium, provided that you conspicuously and    appropriately publish on each copy an appropriate copyright    notice and disclaimer of warranty; keep intact all the notices    that refer to this License and to the absence of any warranty;    and give any other recipients of the Program a copy of this    License along with the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;You may charge a fee for    the physical act of transferring a copy, and you may at your    option offer warranty protection in exchange for a    fee.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;2. You may modify your    copy or copies of the Program or any portion of it, thus    forming a work based on the Program, and copy and distribute    such modifications or work under the terms of Section 1 above,    provided that you also meet all of these conditions:&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;a) You must cause the    modified files to carry prominent notices stating that you    changed the files and the date of any change.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;b) You must cause any work    that you distribute or publish, that in whole or in part    contains or is derived from the Program or any part thereof, to    be licensed as a whole at no charge to all third parties under    the terms of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;c) If the modified program    normally reads commands interactively when run, you must cause    it, when started running for such interactive use in the most    ordinary way, to print or display an announcement including an    appropriate copyright notice and a notice that there is no    warranty (or else, saying that you provide a warranty) and that    users may redistribute the program under these conditions, and    telling the user how to view a copy of this License.     (Exception: if the Program itself is interactive but does not    normally print such an announcement, your work based on the    Program is not required to print an announcement.)&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;These requirements apply    to the modified work as a whole.  If identifiable sections of    that work are not derived from the Program, and can be    reasonably considered independent and separate works in    themselves, then this License, and its terms, do not apply to    those sections when you distribute them as separate works.  But    when you distribute the same sections as part of a whole which    is a work based on the Program, the distribution of the whole    must be on the terms of this License, whose permissions for    other licensees extend to the entire whole, and thus to each    and every part regardless of who wrote it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Thus, it is not the intent    of this section to claim rights or contest your rights to work    written entirely by you; rather, the intent is to exercise the    right to control the distribution of derivative or collective    works based on the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;In addition, mere    aggregation of another work not based on the Program with the    Program (or with a work based on the Program) on a volume of a    storage or distribution medium does not bring the other work    under the scope of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;3. You may copy and    distribute the Program (or a work based on it, under Section 2)    in object code or executable form under the terms of Sections 1    and 2 above provided that you also do one of the    following:&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;a) Accompany it with the    complete corresponding machine-readable source code, which must    be distributed under the terms of Sections 1 and 2 above on a    medium customarily used for software interchange;    or,&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;b) Accompany it with a    written offer, valid for at least three years, to give any    third party, for a charge no more than your cost of physically    performing source distribution, a complete machine-readable    copy of the corresponding source code, to be distributed under    the terms of Sections 1 and 2 above on a medium customarily    used for software interchange; or,&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;c) Accompany it with the    information you received as to the offer to distribute    corresponding source code.  (This alternative is allowed only    for noncommercial distribution and only if you received the    program in object code or executable form with such an offer,    in accord with Subsection b above.)&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The source code for a work    means the preferred form of the work for making modifications    to it.  For an executable work, complete source code means all    the source code for all modules it contains, plus any    associated interface definition files, plus the scripts used to    control compilation and installation of the executable.     However, as a special exception, the source code distributed    need not include anything that is normally distributed (in    either source or binary form) with the major components    (compiler, kernel, and so on) of the operating system on which    the executable runs, unless that component itself accompanies    the executable.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;If distribution of    executable or object code is made by offering access to copy    from a designated place, then offering equivalent access to    copy the source code from the same place counts as distribution    of the source code, even though third parties are not compelled    to copy the source along with the object code.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;4. You may not copy,    modify, sublicense, or distribute the Program except as    expressly provided under this License.  Any attempt otherwise    to copy, modify, sublicense or distribute the Program is void,    and will automatically terminate your rights under this    License. However, parties who have received copies, or rights,    from you under this License will not have their licenses    terminated so long as such parties remain in full    compliance.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;5. You are not required to    accept this License, since you have not signed it.  However,    nothing else grants you permission to modify or distribute the    Program or its derivative works.  These actions are prohibited    by law if you do not accept this License.  Therefore, by    modifying or distributing the Program (or any work based on the    Program), you indicate your acceptance of this License to do    so, and all its terms and conditions for copying, distributing    or modifying the Program or works based on it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;6. Each time you    redistribute the Program (or any work based on the Program),    the recipient automatically receives a license from the    original licensor to copy, distribute or modify the Program    subject to these terms and conditions.  You may not impose any    further restrictions on the recipients&apos; exercise of the rights    granted herein. You are not responsible for enforcing    compliance by third parties to this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;7. If, as a consequence of    a court judgment or allegation of patent infringement or for    any other reason (not limited to patent issues), conditions are    imposed on you (whether by court order, agreement or otherwise)    that contradict the conditions of this License, they do not    excuse you from the conditions of this License.  If you cannot    distribute so as to satisfy simultaneously your obligations    under this License and any other pertinent obligations, then as    a consequence you may not distribute the Program at all.  For    example, if a patent license would not permit royalty-free    redistribution of the Program by all those who receive copies    directly or indirectly through you, then the only way you could    satisfy both it and this License would be to refrain entirely    from distribution of the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;If any portion of this    section is held invalid or unenforceable under any particular    circumstance, the balance of the section is intended to apply    and the section as a whole is intended to apply in other    circumstances.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;It is not the purpose of    this section to induce you to infringe any patents or other    property right claims or to contest validity of any such    claims; this section has the sole purpose of protecting the    integrity of the free software distribution system, which is    implemented by public license practices.  Many people have made    generous contributions to the wide range of software    distributed through that system in reliance on consistent    application of that system; it is up to the author/donor to    decide if he or she is willing to distribute software through    any other system and a licensee cannot impose that    choice.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;This section is intended    to make thoroughly clear what is believed to be a consequence    of the rest of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;8. If the distribution    and/or use of the Program is restricted in certain countries    either by patents or by copyrighted interfaces, the original    copyright holder who places the Program under this License may    add an explicit geographical distribution limitation excluding    those countries, so that distribution is permitted only in or    among countries not thus excluded.  In such case, this License    incorporates the limitation as if written in the body of this    License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;9. The Free Software    Foundation may publish revised and/or new versions of the    General Public License from time to time.  Such new versions    will be similar in spirit to the present version, but may    differ in detail to address new problems or    concerns.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Each version is given a    distinguishing version number.  If the Program specifies a    version number of this License which applies to it and &quot;any    later version&quot;, you have the option of following the terms and    conditions either of that version or of any later version    published by the Free Software Foundation.  If the Program does    not specify a version number of this License, you may choose    any version ever published by the Free Software    Foundation.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;10. If you wish to    incorporate parts of the Program into other free programs whose    distribution conditions are different, write to the author to    ask for permission.  For software which is copyrighted by the    Free Software Foundation, write to the Free Software    Foundation; we sometimes make exceptions for this.  Our    decision will be guided by the two goals of preserving the free    status of all derivatives of our free software and of promoting    the sharing and reuse of software generally.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;NO WARRANTY&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;11. BECAUSE THE PROGRAM IS    LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM,    TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN    OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER    PARTIES PROVIDE THE PROGRAM &quot;AS IS&quot; WITHOUT WARRANTY OF ANY    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A    PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND    PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM    PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY    SERVICING, REPAIR OR CORRECTION.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;12. IN NO EVENT UNLESS    REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY    COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR    REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU    FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR    CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO    USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR    DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR    THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY    OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;END OF TERMS AND    CONDITIONS&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;  &lt;/div&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_license.cpp.rej" line="+7"/>
        <source>&lt;!DOCTYPE html PUBLIC &quot;-//W3C//DTD HTML 4.01 Transitional//EN&quot;&gt;</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>VapourSynthProxy</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/VapourSynth/qt4/vs.ui" line="+14"/>
        <source>VapourSynth Proxy</source>
        <translation></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Select VS file</source>
        <translation>Choisir le fichier VS</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>...</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Run!</source>
        <translation>Lancer</translation>
    </message>
</context>
<context>
    <name>aboutDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/about.ui" line="+16"/>
        <source>About Avidemux</source>
        <translation>A propos d&apos;Avidemux</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>Multi-platform Video Editor</source>
        <translation>Editeur vidéo multiplateformes</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Avidemux </source>
        <translation></translation>
    </message>
    <message>
        <location line="+29"/>
        <source>© 2001 - 2016  Mean</source>
        <translation></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>http://www.avidemux.org</source>
        <translation></translation>
    </message>
    <message>
        <location line="+36"/>
        <source>&amp;License</source>
        <translation></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>OK</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>addBorder</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/addBorder/ADM_vidAddBorder.cpp" line="+182"/>
        <source>_Left border:</source>
        <translation>Bord gauche:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Right border:</source>
        <translation>Bord droit:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Top border:</source>
        <translation>Bord haut:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Bottom border:</source>
        <translation>Bord bas:</translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/addBorder/ADM_vidAddBorder.h" line="+42"/>
        <source>Add Borders</source>
        <translation>Ajout de bord</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Incorrect parameters</source>
        <translation>Paramètres incorrectes</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>All parameters must be even and within range.</source>
        <translation>Tous les paramètres doivent être pairs et dans les bornes.</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/addBorder/ADM_vidAddBorder.h" line="+1"/>
        <source>Add black borders around the image.</source>
        <translation>Ajouter un bord noir autour de l&apos;image</translation>
    </message>
</context>
<context>
    <name>adm</name>
    <message>
        <source>Decode video using VDPAU</source>
        <translation type="obsolete">Utiliser VDPAU pour décoder</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="+203"/>
        <source>Enable openGl support</source>
        <translation>Activer OpenGL</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>_Use systray while encoding</source>
        <translation>Réduire dans la barre de taches</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Accept non-standard audio frequency for DVD</source>
        <translation>Accepter les fréquences non standard pour les DVDs</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Create _OpenDML files</source>
        <translation>Créer des fichiers OpenDML</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enable all SIMD</source>
        <translation>Activer toutes les optimisations SIMD</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable MMX</source>
        <translation>Activer le MMX</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable MMXEXT</source>
        <translation>Activer le MMXEXT</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable 3DNOW</source>
        <translation>Activer le 3DNOW</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable 3DNOWEXT</source>
        <translation>Activer le 3DNOWEXT</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable SSE</source>
        <translation>Activer le SSE</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable SSE2</source>
        <translation>Activer le SSE2</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable SSE3</source>
        <translation>Activer le SSE3</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable SSSE3</source>
        <translation>Activer le SSSE3</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>_lavc threads:</source>
        <translation>Threads lavc:</translation>
    </message>
    <message>
        <location line="+6"/>
        <location filename="../ADM_userInterfaces/ADM_dialog/DIA_xvid4.cpp" line="+47"/>
        <source>High</source>
        <translation>Haute</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Above normal</source>
        <translation>Plus que normale</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normal</source>
        <translation>Normale</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Below normal</source>
        <translation>Sous normale</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../ADM_userInterfaces/ADM_dialog/DIA_xvid4.cpp" line="-2"/>
        <source>Low</source>
        <translation>Basse</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Encoding priority:</source>
        <translation>Priorité de l&apos;encodage:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Indexing/unpacking priority:</source>
        <translation>Priorité de l&apos;indexation:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Playback priority:</source>
        <translation>Priorité de la lecture:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Prioritisation</source>
        <translation>Priorités</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Split MPEG files every (MB):</source>
        <translation>Créer un nouveau fichier MPEG tous les (MB):</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Use alternative tag for MP3 in .mp4</source>
        <translation>Utiliser un tag alternatif pour les pistes mp3 (.mp4)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>XVideo (best)</source>
        <translation>Xvideo (rapide)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>VDPAU (best)</source>
        <translation>VDPAU (très rapide)</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OpenGL (best)</source>
        <translation>OpenGL (rapide)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>SDL (good)</source>
        <translation>SDL (moyen)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Video _display:</source>
        <translation>Affichage vidéo:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>No alerts</source>
        <translation>Pas d&apos;alerte</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display only error alerts</source>
        <translation>N&apos;afficher que les erreurs</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display all alerts</source>
        <translation>Afficher tout</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Message level:</source>
        <translation>Afficher les messages:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>_Volume control:</source>
        <translation>Contrôle du volume:</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>No downmixing</source>
        <translation>Pas de downmixing</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Local playback downmixing:</source>
        <translation>Downmixing pour la lecture:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>_AudioDevice</source>
        <translation>Périphérique Audio</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_postproc.cpp" line="+27"/>
        <source>_Filter strength:</source>
        <translation>Force du filtre:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Swap U and V</source>
        <translation>Inverser U et V</translation>
    </message>
    <message>
        <location line="+10"/>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="+3"/>
        <source>_Horizontal deblocking</source>
        <translation>Deblocking horizontal</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="+1"/>
        <source>_Vertical deblocking</source>
        <translation>Deblocking vertical</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="+2"/>
        <source>_Strength:</source>
        <translation>Force:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Default Postprocessing</source>
        <translation>Postprocessing par défaut</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>User Interface</source>
        <translation>Interface Utilisateur</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Output</source>
        <translation>Sortie</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/myOwnMenu.h" line="+34"/>
        <source>Open</source>
        <translation>Ouvrir</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Append</source>
        <translation>Ajouter</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Save</source>
        <translation>Sauver</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Save as Image</source>
        <translation>Sauver comme image</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Save as BMP</source>
        <translation>Sauver comme BMP</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Save as JPEG</source>
        <translation>Sauver comme JPEG</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Close</source>
        <translation>Fermer</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Connect to avsproxy</source>
        <translation>Se connecter à avsproxy</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Quit</source>
        <translation>Quitter</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Undo</source>
        <translation>Défaire</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Reset Edit</source>
        <translation>Annuler toutes les éditions</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Cut</source>
        <translation>Couper</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Copy</source>
        <translation>Copier</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Paste</source>
        <translation>Coller</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete</source>
        <translation>Effacer</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set Marker A</source>
        <translation>Mettre le marqueur A</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Marker B</source>
        <translation>Mettre le marqueur B</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Decoder Option</source>
        <translation>Option du décodeur</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PostProcessing</source>
        <translation>Post-traitement</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+8"/>
        <source>Filters</source>
        <translation>Filtres</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Select Track</source>
        <translation>Choisir les pistes</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Save audio</source>
        <translation>Sauver la piste audio</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Build Option</source>
        <translation>Options activées</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Advanced</source>
        <translation>&amp;Avancé</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Open Application &amp;Log</source>
        <translation>Ouvrir le fichier de &amp;log</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Open Application Data &amp;Folder</source>
        <translation>Ouvrir le dossier de l&apos;application</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>About</source>
        <translation>A propos</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Play/Stop</source>
        <translation>Jouer/stopper</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Previous Frame</source>
        <translation>Image précédente</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Next Frame</source>
        <translation>Image suivante</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Previous Intra Frame</source>
        <translation>Image clé précédente</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Next Intra Frame</source>
        <translation>Image clé suivante</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Previous Black Frame</source>
        <translation>Image noire précédente</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Next Black Frame</source>
        <translation>Image noire suivante</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>First Frame</source>
        <translation>Première image</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Last Frame</source>
        <translation>Dernière image</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Go To Marker A</source>
        <translation>Aller au marqueur A</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Go To Marker B</source>
        <translation>Aller au marqueur B</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Go To Time</source>
        <translation>Aller au temps</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_audioFilter.cpp" line="+33"/>
        <source>R_esampling (Hz):</source>
        <translation>R_eéchantillonnage (Hz):</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Resampling frequency (Hz)</source>
        <translation>Nouvelle fréquence (Hz)</translation>
    </message>
    <message>
        <location line="+4"/>
        <location line="+25"/>
        <location filename="../ADM_userInterfaces/ADM_dialog/DIA_xvid4.cpp" line="-2"/>
        <source>None</source>
        <translation>Aucun</translation>
    </message>
    <message>
        <location line="-24"/>
        <source>Film to PAL</source>
        <translation>Film vers PAL</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PAL to Film</source>
        <translation>Film vers PAL</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Frame rate change:</source>
        <translation>Changement i/s</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>No change</source>
        <translation>Aucun</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="-77"/>
        <source>Stereo</source>
        <translation>Stéréo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Stereo+surround</source>
        <translation>Stéréo + Surround</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Stereo+center</source>
        <translation>Stéréo+Centre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Stereo+center+surround</source>
        <translation>Stéréo+Centre+Surround</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Stereo front+stereo rear</source>
        <translation>Stéréo Avant+Stéréo Arrière</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>5 channels</source>
        <translation>5 Canaux</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Automatic (max -3 dB)</source>
        <translation>Automatique (max -3 dB)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual (dB)</source>
        <translation>Manuel (dB)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Gain mode:</source>
        <translation>Mode:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>G_ain value:</source>
        <translation>Valeur du gain:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>_Mixer:</source>
        <translation>_Mixeur</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Remix:</source>
        <translation>Remixer:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Shift audio:</source>
        <translation>Décalage audio:</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Shift Value (ms):</source>
        <translation>Valeur (ms):</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Audio Filters</source>
        <translation>Filtres Audio</translation>
    </message>
    <message>
        <location filename="../../common/gui_autodrive.cpp" line="+48"/>
        <source>An audio track is necessary to create such file</source>
        <translation>Une piste audio est nécessaire pour ce type de fichier</translation>
    </message>
    <message>
        <location line="+16"/>
        <location line="+14"/>
        <location line="+31"/>
        <location line="+6"/>
        <location line="+15"/>
        <location line="+27"/>
        <location line="+35"/>
        <source>Codec Error</source>
        <translation>Erreur de codec</translation>
    </message>
    <message>
        <location line="-114"/>
        <source>No AAC audio encoder plugin found.</source>
        <translation>Pas de plugin AAC trouvé.</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>The MP3 codec does not allow disabling reservoir.
Install lame plugin</source>
        <translation>Le codec MP3 n&apos;autorise pas l&apos;option reservoir. Installez le plugin lame</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>You don&apos;t have LAME!.
It is needed to create FLV  video.</source>
        <translation>Vous n&apos;avez pas le plugin LAME. Il est nécessaire pour créer des fichiers FLV.</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Cannot select FLV1  codec.</source>
        <translation>Impossible de choisir le codec FLV1.</translation>
    </message>
    <message>
        <source>Cannot select mpeg4 sp codec.</source>
        <translation type="vanished">Impossible de choisir le codec mpeg4sp.</translation>
    </message>
    <message>
        <location filename="../../common/gui_main.cpp" line="+179"/>
        <source>Select script to save</source>
        <translation>Choisir le script à sauvegarder</translation>
    </message>
    <message>
        <location line="+331"/>
        <source>Are you sure?</source>
        <translation>Etes vous sur ?</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Cannot open &quot;%s&quot;.</source>
        <translation>Impossible d&apos;ouvrir &quot;%s&quot;.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>File error</source>
        <translation>Erreur de fichier</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>&quot;%s&quot; does not exist.</source>
        <translation>&quot;%s&quot; n&apos;existe pas.</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Multiple Audio Tracks</source>
        <translation>Plusieurs pistes audio</translation>
    </message>
    <message>
        <location line="+335"/>
        <location filename="../../../avidemux_core/ADM_coreImage/src/ADM_imageSave.cpp" line="+104"/>
        <source>Something bad happened</source>
        <translation>Quelque chose d&apos;anormal c&apos;est produit</translation>
    </message>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/Q_gui2.cpp.orig" line="+403"/>
        <source>Recent Files</source>
        <translation>Fichiers Récents</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Recent Projects</source>
        <translation>Projets Récents</translation>
    </message>
    <message>
        <location filename="../ADM_UIs/src/T_threadCount.cpp" line="+47"/>
        <location filename="../ADM_UIs/src/T_threadCount.cpp.rej" line="+3"/>
        <source>Custom</source>
        <translation>Manuel</translation>
    </message>
    <message>
        <source>_New frame rate:</source>
        <translation type="obsolete">Nouveau nombre d&apos;i/s:</translation>
    </message>
    <message>
        <source>Interlacing</source>
        <translation type="obsolete">Entrelacement</translation>
    </message>
    <message>
        <source>The filter is already partial</source>
        <translation type="obsolete">The filtre est déjà partiel</translation>
    </message>
    <message>
        <source>Job Name</source>
        <translation type="obsolete">Nom du job</translation>
    </message>
    <message>
        <source>Start Time</source>
        <translation type="obsolete">Date de départ</translation>
    </message>
    <message>
        <source>End Time</source>
        <translation type="obsolete">Date de fin</translation>
    </message>
    <message>
        <source>Sure!</source>
        <translation type="obsolete">Sur!</translation>
    </message>
    <message>
        <source>Delete job</source>
        <translation type="obsolete">Détruire le job</translation>
    </message>
    <message>
        <source>Are you sure you want to delete %s job?</source>
        <translation type="obsolete">Etes vous sur de vouloir détruire %s job ?</translation>
    </message>
    <message>
        <source>Delete *all* job</source>
        <translation type="obsolete">Detruire *tous* les jobs</translation>
    </message>
    <message>
        <source>Are you sure you want to delete ALL jobs?</source>
        <translation type="obsolete">Etes vous sur de vouloir détruire TOUS les jobs ?</translation>
    </message>
    <message>
        <source>Already done</source>
        <translation type="obsolete">Déjà fait</translation>
    </message>
    <message>
        <source>This script has already been successfully executed.</source>
        <translation type="vanished">Ce script a déjà été executé avec succés</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_audioFilter.cpp" line="-41"/>
        <source>Mono</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>5.1</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Dolby Pro Logic</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Dolby Pro Logic II</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>DRC</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Gain</source>
        <translation></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Mixer</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_builtin.cpp" line="+81"/>
        <source>Fontconfig</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>FreeType 2</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Gettext</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>SDL</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>XVideo</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VDPAU</source>
        <translation></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>x86</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>x86-64</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Libraries</source>
        <translation>Librairies</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="+112"/>
        <source>CPU</source>
        <translation></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Built-in Support</source>
        <translation>Options activées</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_gototime.cpp" line="+31"/>
        <source>TimeStamp:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Go to Time</source>
        <translation>Aller au temp</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_jobs_save.cpp" line="+8"/>
        <source>_Job name:</source>
        <translation>Nom du Job</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Output _File:</source>
        <translation>Fichier à écrire:</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Select Video To Write</source>
        <translation>Choisir le fichier</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Save Job</source>
        <translation>Sauver le job</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_plugins.cpp" line="+57"/>
        <source>Audio Plugins</source>
        <translation>Plugins audio</translation>
    </message>
    <message>
        <location line="+23"/>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="-29"/>
        <location line="+3"/>
        <location line="+5"/>
        <location filename="../../common/gui_save.cpp" line="+281"/>
        <location line="+21"/>
        <location line="+7"/>
        <location filename="../../common/gui_savenew.cpp" line="+383"/>
        <source>Audio</source>
        <translation></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Video Encoder Plugins</source>
        <translation>Plugin d&apos;encodage vidéo</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Video Encoder</source>
        <translation>Encoder vidéo</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Video Decoder Plugins</source>
        <translation>Plugin de décodage vidéo</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Video Decoder</source>
        <translation>Décodeur vidéo</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Audio Device Plugins</source>
        <translation>PLugin de périph. Audio</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Audio Device</source>
        <translation>Périph. Audio</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Audio Encoder Plugins</source>
        <translation>Plugins d&apos;encodage Audio</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Audio Encoders</source>
        <translation>Encodeurs audio</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Demuxer Plugins</source>
        <translation>Plugin de lecture</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Demuxers</source>
        <translation>Lecteur</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Muxer Plugins</source>
        <translation>Plugin d&apos;écriture</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Muxers</source>
        <translation>Sortie</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Plugins Info</source>
        <translation>Information plugin</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_postproc.cpp" line="+1"/>
        <source>_Deringing</source>
        <translation>Deringing</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_prefs.cpp" line="-247"/>
        <source>Decode video using VDPAU (NVIDIA)</source>
        <translation>Utiliser VDPAU pour décoder (NVIDIA)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Decode video using XVBA (AMD)</source>
        <translation>Utiliser XVBA pour décoder (AMD)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Decode video using LIBVA (INTEL)</source>
        <translation>Utiliser LIBVA pour décoder (INTEL/...)</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>If you use Hw decoding, it is better to use the matching display driver</source>
        <translation>Si vous activez le décodage HW, il est préférable d&apos;utiliser la même méthode d&apos;affichage</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>_Check for new release</source>
        <translation>Vérifier si il y a de nouvelle version</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>SIMD</source>
        <translation></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Multi-threading</source>
        <translation></translation>
    </message>
    <message>
        <location line="+39"/>
        <source>LIBVA (best)</source>
        <translation></translation>
    </message>
    <message>
        <location line="+28"/>
        <location line="+3"/>
        <source>Sdl driver</source>
        <translation></translation>
    </message>
    <message>
        <location line="+15"/>
        <location filename="../../../avidemux_core/ADM_coreAudio/src/ADM_audioStream.cpp" line="+202"/>
        <source>PCM</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Master</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Pro Logic</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Pro Logic II</source>
        <translation></translation>
    </message>
    <message>
        <location line="+21"/>
        <source>De_ringing</source>
        <translation></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>System language</source>
        <translation>Langue du système</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>_Language</source>
        <translation>Langue</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>_Limit Refresh Rate</source>
        <translation>Limiter le rafraichissement</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Refresh Rate Cap (ms)</source>
        <translation>Limite en ms</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Display</source>
        <translation>Affichage</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>HW Accel</source>
        <translation>Accel. HW</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Threading</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Always ask which port to use</source>
        <translation>Toujours demander le port à utiliser</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Default port to use</source>
        <translation>Port par défaut</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Preferences</source>
        <translation>Préférences</translation>
    </message>
    <message>
        <location filename="../../common/ADM_commonUI/myOwnMenu.h" line="-79"/>
        <location filename="../../common/gui_save.cpp" line="+125"/>
        <location line="+10"/>
        <source>Queue</source>
        <translation>Mettre en queue</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+2"/>
        <location line="+2"/>
        <source>-</source>
        <translation></translation>
    </message>
    <message>
        <location line="-3"/>
        <source>Information</source>
        <translation></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Pr&amp;eferences</source>
        <translation>Préférences</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Save current settings as default</source>
        <translation>Sauver les réglages courants comme défaut</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Plugins</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../common/ADM_editor/src/ADM_segment.cpp" line="+612"/>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_libva.cpp" line="+132"/>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_vdpau_utils.cpp" line="+51"/>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_xvba.cpp" line="+134"/>
        <location filename="../../common/gui_main.cpp" line="+147"/>
        <location line="+27"/>
        <location line="+24"/>
        <location line="+8"/>
        <location line="+4"/>
        <location filename="../../common/gui_save.cpp" line="-64"/>
        <location filename="../../../avidemux_core/ADM_coreUtils/src/ADM_quota.cpp" line="+68"/>
        <source>Error</source>
        <translation>Erreur</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>You cannot remove *all* the video
</source>
        <translation>Vous ne pouvez pas *tout* enlever</translation>
    </message>
    <message>
        <location filename="../../common/ADM_editor/src/utils/ADM_edFrameType.cpp" line="+72"/>
        <source>Updating frametype</source>
        <translation>Mise à jour des informations</translation>
    </message>
    <message>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_libva.cpp" line="+0"/>
        <source>Core has been compiled without LIBVA support, but the application has been compiled with it.
Installation mismatch</source>
        <translation>Avidemux_core n&apos;a pas été compilé avec LIBVA, mais avidemux_app oui. Il y a un problème d&apos;installation.</translation>
    </message>
    <message>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_vdpau_utils.cpp" line="+0"/>
        <source>Core has been compiled without VDPAU support, but the application has been compiled with it.
Installation mismatch</source>
        <translation>Avidemux_core n&apos;a pas été compilé avec VDPAU, mais avidemux_app oui. Il y a un problème d&apos;installation.</translation>
    </message>
    <message>
        <location filename="../../common/ADM_videoCodec/src/ADM_ffmpeg_xvba.cpp" line="+0"/>
        <source>Core has been compiled without XVBA support, but the application has been compiled with it.
Installation mismatch</source>
        <translation>Avidemux_core n&apos;a pas été compilé avec XVBA, mais avidemux_app oui. Il y a un problème d&apos;installation.</translation>
    </message>
    <message>
        <location filename="../../common/gui_autodrive.cpp" line="-82"/>
        <source>No audio track</source>
        <translation>Pas de piste audio</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Cannot select the MPEG-4 SP codec.</source>
        <translation>Impossible de choisir le codec MPEG4 SP</translation>
    </message>
    <message>
        <location line="+93"/>
        <source>Cannot select MPEG-4 SP codec.</source>
        <translation>Impossible de choisir le codec MPEG4 SP</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>You don&apos;t have FAAC!.
It is needed to create PSP compatible video.</source>
        <translation>FAAC n&apos;est pas disponible. 
Il est nécessaire pour créer un fichier compatible PSP</translation>
    </message>
    <message>
        <location filename="../../common/gui_main.cpp" line="-1076"/>
        <source>Select script/project to run</source>
        <translation>Choisir un script/projet à lancer</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Select script to run</source>
        <translation>Choisir un script à lancer</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Select script to debug</source>
        <translation>Choisir un script à débugger</translation>
    </message>
    <message>
        <location line="+154"/>
        <source>Not coded in this version</source>
        <translation>Pas disponible dans cette version</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+65"/>
        <source>Select Video File...</source>
        <translation>Choisir un fichier vidéo</translation>
    </message>
    <message>
        <location line="-40"/>
        <source>No engine</source>
        <translation>Pas de moteur de script</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>tinyPy script is not enabled in this build</source>
        <translation>tinypy n&apos;est pas disponible. C&apos;est anormal.</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Select Video File to Append...</source>
        <translation>Choisir le fichier à concatener</translation>
    </message>
    <message>
        <location line="+122"/>
        <source>Cutting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Error while cutting out.</source>
        <translation>Erreur lors de la coupe</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>Permission error</source>
        <translation>Erreur de permission</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Cannot open project using the video loader.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Try &apos;File&apos; -&gt; &apos;Load/Run Project...&apos;</source>
        <translation>Essayer &quot;Fichier -&gt; lancer un projet&quot;</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Could not open the file</source>
        <translation>Impossible d&apos;ouvrir le fichier</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>The file you just loaded contains several audio tracks.
Go to Audio-&gt;MainTrack to select the active one.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+84"/>
        <source>Something failed when appending</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <location line="+249"/>
        <source>Something bad happened (II)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-56"/>
        <source>Checking video</source>
        <translation>Vérification de la vidéo</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>No error found</source>
        <translation>Pas d&apos;erreur</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Errors found in %u frames</source>
        <translation>Il y a %u frames avec des erreurs</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Marker A &gt; B</source>
        <translation>Le marqueur A est supérieur à B</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot delete the selection.</source>
        <translation>Impossible de detruire la zone séléctionnée</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You can&apos;t remove all frames</source>
        <translation>Vous ne pouvez pas tout détruire</translation>
    </message>
    <message>
        <location line="+96"/>
        <source>_Track from video:</source>
        <translation>_Piste venant de la vidéo</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Main Audio Track</source>
        <translation>Piste audio principale</translation>
    </message>
    <message>
        <location line="+29"/>
        <location line="+7"/>
        <source>Invalid audio index given</source>
        <translation>Piste audio invalide</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Cannot use that file as audio track</source>
        <translation>Impossible d&apos;utiliser ce fichier comme piste audio</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Audio file not found in list, even though it should be there. Create a bug report!</source>
        <translation>Fichier audio non trouvé dans la liste, alors qu&apos;il devrait être là. C&apos;est un bug.</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Unable to set the audio language: No video loaded yet!</source>
        <translation>Impossible de choisir la langue, aucune vidéo chargée</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Setting the language for the given track index is not possible: Video has no audio file!</source>
        <translation>Choisir la langue est impossible, le fichier n&apos;a pas de piste son</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Setting the language for the given track index is not possible: Invalid track index!</source>
        <translation>Impossible de choisir la langue, piste audio invalide</translation>
    </message>
    <message>
        <location line="+133"/>
        <source>AvsProxy</source>
        <translation></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Failed to connect to avsproxy.
Is it running ?</source>
        <translation>Impossible de se connecter à VS/AVS proxy. Est il lancé ?</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>avsproxy</source>
        <translation></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Frame type:</source>
        <translation>Type d&apos;image:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Frame size:</source>
        <translation>Taille de l&apos;image:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Frame Hex Dump</source>
        <translation>Hex dump:</translation>
    </message>
    <message>
        <location line="+108"/>
        <source>Oops</source>
        <translation></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This function is disabled or no longer valid</source>
        <translation>Cette fonction est désactivée ou plus disponible.</translation>
    </message>
    <message>
        <location filename="../../common/gui_play.cpp" line="+411"/>
        <source>Trouble initializing audio device</source>
        <translation>Le device audio ne peut pas être initialisé</translation>
    </message>
    <message>
        <location filename="../../common/gui_save.cpp" line="-326"/>
        <source>No</source>
        <translation>Non</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>No file loaded</source>
        <translation>Pas de fichier chargé</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Job</source>
        <translation></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot reach database. Do you have Job control running ?</source>
        <translation>Impossible de se connecter à la base de données. Avidemux_job est il lancé ?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Output file</source>
        <translation>Fichier de sortie</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Job name</source>
        <translation>Nom du job</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Queue job to jobList</source>
        <translation>Ajouter à la liste de job</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Select Workbench to Save</source>
        <translation>Choisir le projet à sauver</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Select File to Save Audio</source>
        <translation>Choisir le fichier audio à sauver</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Select JPEG Sequence to Save</source>
        <translation>Choisir l&apos;image jpeg à sauver</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Select BMP to Save</source>
        <translation>Choisir l&apos;image BMP à sauver</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Select JPEG to Save</source>
        <translation>Choisir l&apos;image Jpeg à sauver</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Select File to Save</source>
        <translation>Choisir le fichier à écrire</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Saving audio</source>
        <translation>Sauvegarde audio</translation>
    </message>
    <message>
        <location line="+99"/>
        <source>Function not implemented
</source>
        <translation>Fonction non implementée</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Cannot create stream</source>
        <translation>Impossible de créer le flux audio</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Saving failed</source>
        <translation>La sauvegarde a échoué</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Jpeg</source>
        <translation></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Fail to save as jpeg</source>
        <translation>Impossible de sauvegarder le jpeg</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Saving as set of jpegs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Cannot decode frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Aborting.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+38"/>
        <source>Done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-38"/>
        <source>Saved %d images.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Could not save all images.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>BMP op failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Saving %s as a BMP file failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>File %s has been successfully saved.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>File %s was NOT saved correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Cannot get tinyPÿ script engine</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Cannot add job %s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../common/gui_savenew.cpp" line="-211"/>
        <source>Reuse previous first pass data ?
Warning, the settings must be close.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+138"/>
        <location line="+12"/>
        <location line="+16"/>
        <location line="+8"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Cannot instantiate video chain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <location line="+24"/>
        <source>Cannot create encoder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-8"/>
        <source>Cannot setup codec. Bitrate too low?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Cannot setup audio encoder, make sure your stream is compatible with audio encoder (number of channels, bitrate, format)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>The video is in copy mode but the cut points are not on keyframes.
The video will be saved but there will be corruption at cut point(s).
Do you want to continue anyway ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+26"/>
        <source>Muxer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-26"/>
        <source>Cannot instantiate muxer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Cannot open </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ADM_UIs/src/T_threadCount.cpp" line="-2"/>
        <location filename="../ADM_UIs/src/T_threadCount.cpp.rej" line="-2"/>
        <source>Disabled</source>
        <translation type="unfinished">Désactivé</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../ADM_UIs/src/T_threadCount.cpp.rej" line="+1"/>
        <source>Auto-detect</source>
        <translation type="unfinished">Détection auto</translation>
    </message>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/DIA_xvid4.cpp" line="-9"/>
        <source>_Interlaced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ca_rtoon mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Greyscale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Turbo mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>C_hroma optimizer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Main</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Very Low</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Medium</source>
        <translation type="unfinished">Moyenne</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Very High</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ultra High</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Motion Search Precision</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mode Decision</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Limited Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Medium Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Wide Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VHQ Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Max B Frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quarter Pixel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>GMC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>BVHQ</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Advanced Simple Profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>4MV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Chroma ME</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>HQ AC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>More Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Min Gop Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max Gop Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>GOP Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Motion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H263</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mpeg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantization Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Trellis Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Two Pass Tuning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Key Frame Boost(%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>I-frames closer than...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>.. are reduced by(%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max Overflow Improvement(%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max Overglow Degradation(%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Curve Compression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>High Bitrate Scenes (%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Bitrate Scenes (%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Overflow Control Strength</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Two Pass</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Xvid4 Configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_core/src/ADM_prettyPrint.cpp" line="+23"/>
        <source>%d minute(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>%d hour(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Less than a minute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>A few seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreAudio/src/ADM_audioStream.cpp" line="-1"/>
        <source>DTS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>MP2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MP3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>WMAPRO</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>WMA</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>LPCM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AC3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>OPUS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>E-AC3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ogg Vorbis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MP4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AAC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>QDM2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AMR-NB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MSADPCM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>ULAW</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>IMA ADPCM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>8-bit PCM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unknown codec</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreAudio/src/ADM_audioStreamMP3.cpp" line="+176"/>
        <source>Building time map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreImage/src/ADM_imageSave.cpp" line="-26"/>
        <source>Memory error</source>
        <translation type="unfinished">Impossible d&apos;allouer la mémoire</translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreMuxer/src/ADM_coreMuxerFfmpeg.cpp" line="+425"/>
        <source>Saving</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+132"/>
        <source>Too short</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>The video has been saved but seems to be incomplete.</source>
        <translation>La vidéo a été sauvegardée mais semble incompléte</translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreUtils/src/ADM_iso639.cpp" line="+17"/>
        <location filename="../../../avidemux_core/ADM_coreUtils/src/avidemutils.cpp" line="+384"/>
        <source>Unknown</source>
        <translation>Inconnu</translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreUtils/src/ADM_quota.cpp" line="-4"/>
        <source>can&apos;t open &quot;%s&quot;: %s
%s
</source>
        <translation>Impossible d&apos;ouvrir &quot;%s&quot;: %s
%s
</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+85"/>
        <source>filesystem full</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-85"/>
        <location line="+85"/>
        <source>quota exceeded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-84"/>
        <location line="+85"/>
        <source>Please free up some space and press RETRY to try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-78"/>
        <source>can&apos;t open &quot;%s&quot;: %u (%s)
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+75"/>
        <source>can&apos;t write to file &quot;%s&quot;: %s
%s
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+11"/>
        <source>__unknown__</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-8"/>
        <source>Ignore</source>
        <translation>Ignorer</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Retry</source>
        <translation>Re-essayer</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>can&apos;t write to file &quot;%s&quot;: %u (%s)
</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreUtils/src/avidemutils.cpp" line="-7"/>
        <source>NTSC 4:3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1"/>
        <source>NTSC 16:9</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PAL 4:3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PAL 16:9</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>1:1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreVideoCodec/include/ADM_ffmp43.h" line="+156"/>
        <location line="+32"/>
        <location filename="../../../avidemux_core/ADM_coreVideoCodec/include/ADM_ffmp43.h.rej" line="+8"/>
        <location line="+9"/>
        <location filename="../../../avidemux_core/ADM_coreVideoCodec/src/ADM_codecFFsimple.cpp" line="+34"/>
        <location line="+33"/>
        <source>Codec</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_core/ADM_coreVideoCodec/src/ADM_codecFFsimple.cpp" line="-33"/>
        <source>Internal error finding codec 0x%x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Internal error opening 0x%x</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>aften</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/aften/audioencoder_aften.cpp" line="+246"/>
        <source>Aften Configuration</source>
        <translation>Configuration d&apos;Aften</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>asfdemuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Asf/ADM_asf.cpp" line="+158"/>
        <source>File Error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot open file
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Asf/ADM_asfHeaders.cpp" line="+471"/>
        <source>Indexing</source>
        <translation type="unfinished">Indexation</translation>
    </message>
</context>
<context>
    <name>asharp</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/asharp/ADM_vidAsharp.cpp" line="+65"/>
        <source>Asharp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Adaptative sharpener by MarcFD.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/asharp/DIA_flyAsharp.cpp" line="+107"/>
        <source>Original</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Processed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>asharpDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/asharp/qt4/asharp.ui" line="+13"/>
        <source>ASharp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+96"/>
        <source>Strength</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Threshold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Block Adaptative</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Unknown flag</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ass</name>
    <message>
        <source>Hardcode ass/ssa subtitles using libass.</source>
        <translation type="vanished">Incruster des sous titres ASS/SSA.</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/ass/ADM_vidASS.cpp" line="+64"/>
        <source>Hardcode ass/ssa/srt subtitles using libass.</source>
        <translation type="unfinished">Incruster des sous titres SSA/ASS avec libass</translation>
    </message>
    <message>
        <location line="+103"/>
        <source>_Subtitle file (ASS/SSA):</source>
        <translation>Fichier de sous-titres (Ass/Ssa):</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Select Subtitle file</source>
        <translation>Sélectionner sous titres</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Line spacing:</source>
        <translation>Inter-Lignes:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Font scale:</source>
        <translation>Taille de la fonte:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Top margin:</source>
        <translation>Marge haute:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Botto_m margin</source>
        <translation>Marge basse </translation>
    </message>
    <message>
        <location line="-108"/>
        <source>SSA/ASS/SRT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+63"/>
        <source>Format ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Are you sure this is an ass file ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+49"/>
        <source>ASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>This is a srt file. Convert to SSA ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <location line="+5"/>
        <location line="+8"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="-13"/>
        <source>Cannot load this srt file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Cannot convert to ssa.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Cannot save converted file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Fonts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Preparing the fonts can take a few minutes the first time.
This message will not be displayed again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+27"/>
        <source>SSA Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot read_file for *%s*</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>avimuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerAvi/muxerAvi.cpp" line="+74"/>
        <source>Bad Idea</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Using H264/H265 in AVI is a bad idea, MKV is better for that.
 Do you want to continue anyway ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot create AVI file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerAvi/muxerAviConfig.cpp" line="+25"/>
        <source>Muxing Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Avi Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>avsfilter</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/avsfilter/avsfilter.cpp" line="+599"/>
        <source>Select wine filename[wine/cedega/etc.]</source>
        <translation type="unfinished">Choisir le nom de l&apos;éxécutable wine (wine/cedega/...)</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Select avs filename[*.avs]</source>
        <translation type="unfinished">Choisir le fichier avisynth (*.avs)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>AvsFilter config</source>
        <translation type="unfinished">Configuration AVSFilter</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>_wine app file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_loader file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select loader filename[avsload.exe]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_avs file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_pipe timeout:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>bitrate</name>
    <message>
        <location filename="../ADM_UIs/src/T_bitrate.cpp" line="+50"/>
        <source>Constant Bitrate</source>
        <translation type="unfinished">Bitrate constant</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Constant Quantiser</source>
        <translation type="unfinished">Quantisation constante</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Same Quantiser as Input</source>
        <translation type="unfinished">Même quantisation que la source</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Constant Rate Factor</source>
        <translation type="unfinished">Rate Factor constant</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Two Pass - Video Size</source>
        <translation type="unfinished">2 Passes- Taille de la vidéo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Two Pass - Average Bitrate</source>
        <translation type="unfinished">2 Passes - Bitrate moyen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Encoding mode</source>
        <translation type="unfinished">Mode d&apos;encodage</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Bitrate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+86"/>
        <source>Target bitrate (kb/s)</source>
        <translation type="unfinished">Bitrate cible (kb/s)</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+19"/>
        <source>Quantizer</source>
        <translation type="unfinished">Quantisation</translation>
    </message>
    <message>
        <location line="-14"/>
        <source>Target video size (MB)</source>
        <translation type="unfinished">Taille cible de la vidéo (MB)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Average bitrate (kb/s)</source>
        <translation type="unfinished">Bitrate moyen (kb/s)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>black</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/black/ADM_vidBlack.cpp" line="+49"/>
        <source>Replace a section by black.</source>
        <translation type="unfinished">Remplacer une portion de la video par du noir</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Replace by Black</source>
        <translation type="unfinished">Remplacer par du noir</translation>
    </message>
    <message>
        <location line="-16"/>
        <source>Black</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>_Start time (ms):</source>
        <translation type="unfinished">Début (ms):</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_End time (ms):</source>
        <translation type="unfinished">Fin(ms):</translation>
    </message>
</context>
<context>
    <name>blacken</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/blackenBorder/ADM_vidBlackBorder.cpp" line="+159"/>
        <source>_Left border:</source>
        <translation>Bord gauche:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Right border:</source>
        <translation>Bord droit:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Top border:</source>
        <translation>Bord haut:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Bottom border:</source>
        <translation>Bord bas:</translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/blackenBorder/ADM_vidBlackBorder.h" line="+43"/>
        <source>Blacken Borders</source>
        <translation>Noircir les bords</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Incorrect parameters</source>
        <translation>Paramètres incorrectes</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>All parameters must be even and within range.</source>
        <translation>Tous les paramètres doivent être pairs et dans les bornes.</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/blackenBorder/ADM_vidBlackBorder.h" line="+1"/>
        <source>Remove noisy edge by turning them to black.</source>
        <translation type="unfinished">Supprime les bords endommagés en les remplacant par du noir</translation>
    </message>
</context>
<context>
    <name>blackframes</name>
    <message>
        <location filename="../../common/gui_blackframes.cpp" line="+105"/>
        <source>BlackFrame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This function is unsupported at the moment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Searching black frame..</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>changeFps</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/changeFps/changeFps.cpp" line="+40"/>
        <source>Custom</source>
        <translation>Manuel</translation>
    </message>
    <message>
        <location line="+168"/>
        <source>Source Fps:</source>
        <translation type="unfinished">Source:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Source frame rate:</source>
        <translation>i/s source:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Destination Fps:</source>
        <translation type="unfinished">Destination:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Destination frame rate:</source>
        <translation>i/s destination:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Change fps</source>
        <translation>Changer le nombre d&apos;i/s</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Invalid fps</source>
        <translation>i/s invalide</translation>
    </message>
    <message>
        <source>_New frame rate:</source>
        <translation type="obsolete">Nouveau nombre d&apos;i/s:</translation>
    </message>
    <message>
        <location line="-186"/>
        <source>25  (PAL)</source>
        <translation type="unfinished">25  (Pal/Secam)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>23.976 (Film)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>29.97 (NTSC)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>50 (Pal)</source>
        <translation type="unfinished">50 (Pal/Secam)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>59.93  (NTSC)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Change FPS</source>
        <translation type="unfinished">Changer le nombre d&apos; I/S</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Speed up/slow down the video as if altering fps. This filter changes duration.</source>
        <translation type="unfinished">Accélère/Ralentit la vidéo en modifiant le nombre d&apos;i/s. La durée est modifiée.</translation>
    </message>
    <message>
        <location line="+150"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
</context>
<context>
    <name>chromashift</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/chromaShift/ADM_vidChromaShift.cpp" line="+34"/>
        <source>ChromaShift</source>
        <translation type="unfinished">Decalage chroma</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Shift chroma U/V to fix badly synced luma/chroma.</source>
        <translation type="unfinished">Decaler la chroma u/v pour corriger les couleurs non alignées</translation>
    </message>
</context>
<context>
    <name>chromashiftDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/chromaShift/qt4/chromashift.ui" line="+13"/>
        <source>ChromaShift</source>
        <translation type="unfinished">Decalage chroma</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>V Shift</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>U Shift</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>coloryuv</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/colorYUV/ADM_vidColorYuv.cpp" line="+78"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="-16"/>
        <source>Avisynth color filter.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Color management filter.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>AutoWhite</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AutoGain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Clip to Tv Range (16-235)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>PC-&gt;TV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>TV-&gt;PC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Levels:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Y gain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y Brightness</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y Gamma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>U gain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>U Brightness</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>U Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>V gain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>V Brightness</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>V Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Flags</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>U</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>colorYuv</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>contrast</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/contrast/ADM_vidContrast.cpp" line="+34"/>
        <source>Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Adjust contrast, brightness and colors.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>contrastDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/contrast/qt4/contrast.ui" line="+13"/>
        <location line="+117"/>
        <source>Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-40"/>
        <source>Luma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>ChromaU</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>ChromaV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Brightness</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>convolution</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fastConvolution/ADM_vidFastConvolution.cpp" line="+136"/>
        <source>_Process luma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Process luma plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>P_rocess chroma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Fast Convolution</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>crash</name>
    <message>
        <location filename="../../common/ADM_osSupport/ADM_crashHook.cpp" line="+85"/>
        <source>Load it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Crash file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>I have detected a crash file. 
Do you want to load it  ?
(It will be deleted in all cases, you should save it if you want to keep it)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>crop</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/crop/ADM_vidCrop.cpp" line="+56"/>
        <source>crop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>crop filter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Warning Cropping too much width ! Width reseted !
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Warning Cropping too much height ! Height reseted !
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>cropDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/crop/qt4/crop.ui" line="+13"/>
        <source>Crop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Right:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Top:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Reset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Bottom:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Left:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Auto Crop</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dcaenc</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/dcaenc/audioencoder_dcaenc.cpp" line="+272"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>DcaEnc Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>decimate</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/decimate/decimate.cpp" line="+74"/>
        <source>Discard closer</source>
        <translation type="unfinished">Eliminer le plus proche</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Replace (interpolate)</source>
        <translation type="unfinished">Remplacer (interpoler)</translation>
    </message>
    <message>
        <source>Discard longer dupe (animÃ©s)</source>
        <translation type="obsolete">Eliminer la séquence répétée la plus longue (animés)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pulldown dupe removal</source>
        <translation type="unfinished">Elimination du pulldown</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Fastest (no chroma, partial luma)</source>
        <translation type="unfinished">Rapide (luma partielle)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Medium (full luma, no chroma)</source>
        <translation type="unfinished">Moyen (toute la luma)</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>_Quality:</source>
        <translation type="unfinished">_Qualité:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Threshold 1:</source>
        <translation type="unfinished">_Seuil 1:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>T_hreshold 2:</source>
        <translation type="unfinished">S_euil2:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Sho_w</source>
        <translation type="unfinished">A_fficher</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Decomb decimate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Donald Graft decimate. Remove duplicate after telecide.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Discard longer dupe (animés)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>C_ycle:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Decomb Decimate</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>delogo2</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mplayerDelogo/ADM_vidMPdelogo.cpp" line="+42"/>
        <source>MPlayer delogo2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Blend a logo by interpolating its surrounding box.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dgbob</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/dgBob/ADM_vidDGbob.cpp" line="+72"/>
        <source>Keep nb of frames and fps</source>
        <translation type="unfinished">Garder les images et le nombre d&apos;i/s</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Double nb of frames and fps</source>
        <translation type="unfinished">Doubler les images et le nombre d&apos;i/s</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Double nb of frames (slow motion)</source>
        <translation type="unfinished">Doubler le nombre d&apos;images (ralenti)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Threshold:</source>
        <translation type="unfinished">_Seuil:</translation>
    </message>
    <message>
        <location line="-23"/>
        <source>dgbob</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Donald Graft Bob.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>_Top Field First:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Extra</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Extra check, avoid using it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>DGBob</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dummy</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/dummy/dummyVideoFilter.cpp" line="+44"/>
        <source>Dummy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Null filter, it does nothing at all.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dv</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffDv/ADM_ffDv.cpp" line="+79"/>
        <source>DV only supports 720*576*25fps and 720*480*29.97fps</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>encodingDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/encoding.ui" line="+26"/>
        <source>Encoding...</source>
        <translation type="unfinished">Encodage...</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>Phase:</source>
        <translation type="unfinished">Phases:</translation>
    </message>
    <message>
        <location line="+216"/>
        <source>Video Codec:</source>
        <translation type="unfinished">Codec Vidéo:</translation>
    </message>
    <message>
        <location line="-229"/>
        <location line="+182"/>
        <location line="+24"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="-223"/>
        <location line="+216"/>
        <source>Unknown</source>
        <translation type="unfinished">Inconnu</translation>
    </message>
    <message>
        <location line="-145"/>
        <source>Priority:</source>
        <translation type="unfinished">Priorité:</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>High</source>
        <translation type="unfinished">Haute</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Above Normal</source>
        <translation type="unfinished">Plus que normale</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Normal</source>
        <translation type="unfinished">Normale</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Below Normal</source>
        <translation type="unfinished">Sous normale</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Low</source>
        <translation type="unfinished">BAsse</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Pause / Abort</source>
        <translation type="unfinished">Pause/Abandon</translation>
    </message>
    <message>
        <location line="-151"/>
        <source>Main</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+48"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Time Remaining:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Shut down computer when finished</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+102"/>
        <source>Advanced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+53"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Audio Codec:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Container:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+50"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Audio Size:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Total Size:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+7"/>
        <location line="+17"/>
        <source>0 MB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-10"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Video Size:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+47"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Average Bitrate:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+37"/>
        <location line="+51"/>
        <source>0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-78"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Processed Frames:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Quantiser:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>0 kB/s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+41"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Elapsed:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>00:00:00</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:8pt; font-weight:600;&quot;&gt;Frames/sec:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>eq2</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/eq2/ADM_vidEq2.cpp" line="+59"/>
        <source>MPlayer eq2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Adjust contrast, brightness, saturation and gamma.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>eq2Dialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/eq2/qt4/eq2.ui" line="+13"/>
        <location line="+104"/>
        <source>Contrast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-44"/>
        <source>&lt;b&gt;Gamma&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Saturation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Brightness</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+95"/>
        <source>Blue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Red</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Initial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Green</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>faac</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/faac/audioencoder_faac.cpp" line="+292"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Faac Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>fade</name>
    <message>
        <source>_Start time (ms):</source>
        <translation type="obsolete">Début (ms):</translation>
    </message>
    <message>
        <source>_End time (ms):</source>
        <translation type="obsolete">Fin(ms):</translation>
    </message>
    <message>
        <source>Fade to black</source>
        <translation type="obsolete">Fondu au noir</translation>
    </message>
</context>
<context>
    <name>fadeTo</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fadeTo/ADM_vidFadeTo.cpp" line="+54"/>
        <location line="+24"/>
        <source>Fade</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-23"/>
        <source>Fade.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>_Start time (ms):</source>
        <translation type="unfinished">Début (ms):</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_End time (ms):</source>
        <translation type="unfinished">Fin(ms):</translation>
    </message>
</context>
<context>
    <name>fadeToBlack</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fadeToBlack/ADM_vidFade.cpp" line="+53"/>
        <location line="+26"/>
        <source>Fade to black</source>
        <translation type="unfinished">Fondu au noir</translation>
    </message>
    <message>
        <location line="-25"/>
        <source>Fade to black in/out.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Fade out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Fade in</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>_Fade type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Start time (ms):</source>
        <translation type="unfinished">Début (ms):</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_End time (ms):</source>
        <translation type="unfinished">Fin(ms):</translation>
    </message>
</context>
<context>
    <name>ffmpeg2</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffMpeg2/ADM_ffMpeg2.cpp" line="+295"/>
        <source>Interlacing</source>
        <translation type="unfinished">Entrelacement</translation>
    </message>
    <message>
        <location line="-76"/>
        <source>Normal (4:3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Wide (16:9)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tmpgenc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Animes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>KVCD</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>MB comparison</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Fewest bits (vhq)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rate distortion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>One thread</source>
        <translation type="unfinished">Un thread</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Two threads)</source>
        <translation type="unfinished">Deux threads</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Three threads</source>
        <translation type="unfinished">Trois threads</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Auto (#cpu)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Progressive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Interlaced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Top Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Threading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mi_n. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ma_x. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max. quantizer _difference:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VBV Buffer Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max bitrate (kb/s):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Trellis quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Number of B frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Macroblock decision:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Aspect ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Matrices:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Filesize tolerance (kb):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quantizer compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantizer _blur:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Gop Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Interlaced:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Field Order:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Basic Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Adv. Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>libavcodec MPEG-2 configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ffmpeg4</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffMpeg4/ADM_ffMpeg4.cpp" line="+189"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+58"/>
        <source>User Interface</source>
        <translation type="unfinished">Interface Utilisateur</translation>
    </message>
    <message>
        <location line="-57"/>
        <source>Full</source>
        <translation type="unfinished">Complète</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Phods</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>EPZS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>X1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>H.263</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MPEG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>MB comparison</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Fewest bits (vhq)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rate distortion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>One thread</source>
        <translation type="unfinished">Un thread</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Two threads)</source>
        <translation type="unfinished">Deux threads</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Three threads</source>
        <translation type="unfinished">Trois threads</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Auto (#cpu)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Matrices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Threading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mi_n. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ma_x. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max. quantizer _difference:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>4_MV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Trellis quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quarter pixel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_GMC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Number of B frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Quantization type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Macroblock decision:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Filesize tolerance (kb):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quantizer compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantizer _blur:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Gop Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Advanced Simple Profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Motion Estimation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>libavcodec MPEG-4 configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ffmsmpeg4</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffMsMpeg4/ADM_ffMsMp4.cpp" line="+225"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>User Interface</source>
        <translation type="unfinished">Interface Utilisateur</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Full</source>
        <translation type="unfinished">Complète</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Phods</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>EPZS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>X1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>H.263</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MPEG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>MB comparison</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Fewest bits (vhq)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rate distortion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Mi_n. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ma_x. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max. quantizer _difference:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Trellis quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Filesize tolerance (kb):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quantizer compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantizer _blur:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Gop Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>libavcodec MPEG-4 configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ffnvenc</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffNvEnc/ADM_ffNvEnc.cpp" line="+194"/>
        <source>Low Quality</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>High Quality</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>BluRay</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency (LQ)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency (HQ)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Preset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bitrate (kbps):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max Bitrate (kbps):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>libavcodec MPEG-4 configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ffpsmuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerffPS/muxerffPS.cpp" line="+64"/>
        <source>[Mismatch]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Saving mpeg PS (ff)</source>
        <translation type="unfinished">Sauvegarde au format PS (ff)</translation>
    </message>
    <message>
        <location line="+45"/>
        <source> video not compatible
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source> Bad width/height for VCD
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source> Bad width/height for SVCD
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source> Bad width/height for DVD
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+17"/>
        <source> VCD : only MP2 audio accepted
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source> VCD : only 44.1 khz audio accepted
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source> DVD : only 48 khz audio accepted
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>[ffPS] DVD : only MP2/AC3/DTS audio accepted
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerffPS/muxerffPSConfig.cpp" line="+51"/>
        <source>Free</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Muxing Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Allow non compliant stream</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Total Muxrate (kbits)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Video Muxrate (kbits)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VBV size (kBytes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Advanced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Mpeg PS Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>fftsmuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerffTS/muxerffTS.cpp" line="+141"/>
        <source>Saving mpeg TS (ff)</source>
        <translation type="unfinished">Sauvegarde au format TS (ff)</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerffTS/muxerffTSConfig.cpp" line="+28"/>
        <source>VBR muxing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mux rate (MBits/s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>TS Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>filesel</name>
    <message>
        <location filename="../../../avidemux_core/ADM_coreUI/src/DIA_fileSel.cpp" line="+150"/>
        <source>File error</source>
        <translation type="unfinished">Erreur de fichier</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cannot open &quot;%s&quot;.</source>
        <translation type="unfinished">Impossible d&apos;ouvrir &quot;%s&quot;.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>%s already exists.

Do you want to replace it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+14"/>
        <source>It is possible that you are trying to overwrite an input file!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Cannot write the file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>No write access to &quot;%s&quot;.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>filtermainWindow</name>
    <message>
        <source>The filter is already partial</source>
        <translation type="obsolete">The filtre est déjà partiel</translation>
    </message>
</context>
<context>
    <name>flux</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fluxSmooth/ADM_vidFlux.cpp" line="+88"/>
        <source>_Temporal threshold:</source>
        <translation type="unfinished">Seuil temporel:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Spatial threshold:</source>
        <translation type="unfinished">Seuil spacial:</translation>
    </message>
    <message>
        <location line="-52"/>
        <location line="+56"/>
        <source>FluxSmooth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-55"/>
        <source>Spatio-temporal cleaner by Ross Thomas.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>flv1</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/ffFlv1/ADM_ffFlv1.cpp" line="+163"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>User Interface</source>
        <translation type="unfinished">Interface Utilisateur</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Full</source>
        <translation type="unfinished">Complète</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Phods</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>EPZS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>X1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>H.263</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MPEG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>MB comparison</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Fewest bits (vhq)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rate distortion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Mi_n. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ma_x. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max. quantizer _difference:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Trellis quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Filesize tolerance (kb):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quantizer compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantizer _blur:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Gop Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>libavcodec FLV1 configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>flvdemuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Flv/ADM_flv.cpp" line="+405"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This FLV file says it has no video.
I will assume it has and try to continue</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>flvmuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerFlv/muxerFlv.cpp" line="+68"/>
        <location line="+10"/>
        <location line="+6"/>
        <source>Unsupported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-16"/>
        <source>Only FLV1 &amp; VP6 supported for video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Only AAC &amp; mpegaudio supported for audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only 44.1, 22.050 and 11.025 kHz supported</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>gaussian</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fastConvolution/Gauss.cpp" line="+26"/>
        <source>Gaussian convolution.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>3x3 convolution filter :gaussian.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glBenchmark</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glBenchmark/benchGl.cpp" line="+61"/>
        <source>OpenGl ReadBack benchmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Check how fast readback is.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glFragment</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.cpp" line="+68"/>
        <source>OpenGl Fragment Shader Sample</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run a fragment shader.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glFragment2</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.cpp" line="+66"/>
        <source>OpenGl Fragment Shader Sample2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run a fragment shader.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glResize</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.cpp" line="+66"/>
        <source>OpenGl Resize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Resize using openGl.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+150"/>
        <source>Width :</source>
        <translation type="unfinished">Largeur:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Height :</source>
        <translation type="unfinished">Hauteur:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>glResize</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glRotate</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.cpp" line="+57"/>
        <source>OpenGl Rotate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rotate image by a small amount.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+120"/>
        <source>Angle (°):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>glRotate</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glSample</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.cpp" line="+68"/>
        <source>OpenGl Fragment Shader Sample</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run a fragment shader.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glShader</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glShaderLoader/shader.cpp" line="+60"/>
        <source>Shader Loader</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run an external shader program.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+164"/>
        <source>ShaderFile to load</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>ShaderLoader</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glSmooth</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.cpp" line="+61"/>
        <source>OpenGl Smooth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Smooth image while preserving edge.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glVdpaufilter</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vidVdpauFilterDeint.cpp" line="+84"/>
        <source>vdpauDeintGl</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VDPAU deinterlacer+resize, openGl version (faster).</source>
        <translation type="unfinished">VDPAU de-entrelace+resize, openGl version (faster).</translation>
    </message>
    <message>
        <location line="+178"/>
        <source>Keep Top Field</source>
        <translation type="unfinished">Garder le champs supérieur</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Keep Bottom Field</source>
        <translation type="unfinished">Garder le champs inférieur</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Double framerate</source>
        <translation type="unfinished">Doubler le nombre d&apos;image par seconde</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Resize:</source>
        <translation type="unfinished">_Redimensionner</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Deint Mode:</source>
        <translation type="unfinished">Mode de _deentrelacement:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Width :</source>
        <translation type="unfinished">Largeur:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Height :</source>
        <translation type="unfinished">Hauteur:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>vdpau</source>
        <translation type="unfinished">vdpau</translation>
    </message>
</context>
<context>
    <name>glVertex</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.cpp" line="+59"/>
        <source>OpenGl Vertex Shader</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run a simple vertex shader.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glWave</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.cpp" line="+59"/>
        <source>OpenGl wave </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Simple wave filter.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>glYadif</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.cpp" line="+77"/>
        <source>Yadif (openGl)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Yet another deinterlacer, using shaders.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+182"/>
        <source>Temporal &amp; spatial check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bob, temporal &amp; spatial check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Skip spatial temporal check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bob, skip spatial temporal check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Bottom field first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Top field first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Order:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>yadif</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>hflip</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/horizontalFlip/horizontalFlip.cpp" line="+44"/>
        <source>Horizontal Flip</source>
        <translation>Inversion horizontale</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Horizontally flip the image.</source>
        <translation>Inverse l&apos;image horizontalement.</translation>
    </message>
</context>
<context>
    <name>histogram</name>
    <message>
        <location filename="../../common/ADM_commonUI/DIA_bitrateHisto.cpp" line="+56"/>
        <location line="+75"/>
        <source>No data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Max. bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Average bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Number of I frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Number of P frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Number of B frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max. B frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Bitrate Histogram</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>hue</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/hue/ADM_vidHue.cpp" line="+57"/>
        <source>Mplayer Hue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Adjust hue and saturation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/hue/DIA_flyHue.cpp" line="+61"/>
        <source>Original</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Processed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>hueDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/hue/qt4/hue.ui" line="+13"/>
        <location line="+90"/>
        <source>Hue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Saturation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>huff</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/huff/ADM_huffEncoder.cpp" line="+104"/>
        <source>HUFFYUV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>FF HUFFYUV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>HuffYUV Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>hzstackfield</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/stackField/hzStackField.cpp" line="+52"/>
        <source>Horizontal Stack Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Put fields side by side.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>indexing</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/T_index_pg.cpp" line="+92"/>
        <source>Time Left :%02d:%02d:%02d</source>
        <translation type="unfinished">Temps restant :%02d:%02d:%02d</translation>
    </message>
    <message>
        <location line="-76"/>
        <source>Continue indexing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Abort Requested</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Do you want to abort indexing ?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ivtcRemover</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/ivtcDupeRemover/ADM_ivtcDupeRemover.cpp" line="+81"/>
        <source>Remove IVTC dupe.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove the duplicate frames present after ivtc.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+296"/>
        <source>_Noise:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Show:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Full</source>
        <translation type="unfinished">Complète</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Fast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VeryFast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Frame rate change:</source>
        <translation type="unfinished">Changement i/s</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>DupeRemover</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>jobs</name>
    <message>
        <location filename="../ADM_jobs/src/uiJobs.ui" line="+28"/>
        <source>Run all pending jobs</source>
        <translation type="unfinished">Executer tous les jobs en attente</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Run jobs</source>
        <translation type="unfinished">Executer</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Delete already executed jobs</source>
        <translation type="unfinished">Effacer les jobs déjà faits</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Cleanup</source>
        <translation type="unfinished">Nettoyer</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>If you use VDPAU you cannot use CLI version</source>
        <translation type="unfinished">Si vous utilisez VDAU, la version CLI ne marchera pas</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>use QT4 version</source>
        <translation type="unfinished">Utiliser la version Qt4</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Quit</source>
        <translation type="unfinished">Quitter</translation>
    </message>
    <message>
        <location line="-54"/>
        <source>Avidemux Jobs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../common/GUI_jobs.cpp" line="+37"/>
        <source>Oops</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Something very wrong happened when building joblist.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>There are no jobs stored</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>jobsDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/jobs.ui" line="+13"/>
        <source>Jobs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Delete Sel. Job</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Delete All Jobs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Run All Jobs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Run Selected Job</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>jobsWindow</name>
    <message>
        <source>Job Name</source>
        <translation type="obsolete">Nom du job</translation>
    </message>
    <message>
        <source>Start Time</source>
        <translation type="obsolete">Date de départ</translation>
    </message>
    <message>
        <source>End Time</source>
        <translation type="obsolete">Date de fin</translation>
    </message>
    <message>
        <source>Sure!</source>
        <translation type="obsolete">Sur!</translation>
    </message>
    <message>
        <source>Delete job</source>
        <translation type="obsolete">Détruire le job</translation>
    </message>
    <message>
        <source>Are you sure you want to delete %s job?</source>
        <translation type="obsolete">Etes vous sur de vouloir détruire %s job ?</translation>
    </message>
    <message>
        <source>Delete *all* job</source>
        <translation type="obsolete">Detruire *tous* les jobs</translation>
    </message>
    <message>
        <source>Are you sure you want to delete ALL jobs?</source>
        <translation type="obsolete">Etes vous sur de vouloir détruire TOUS les jobs ?</translation>
    </message>
    <message>
        <source>Already done</source>
        <translation type="obsolete">Déjà fait</translation>
    </message>
    <message>
        <source>This script has already been successfully executed.</source>
        <translation type="obsolete">Ce script a déjà été executé avec succés</translation>
    </message>
</context>
<context>
    <name>jpeg</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/jpeg/ADM_jpegEncoder.cpp" line="+83"/>
        <source>YUV422</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>YUV420</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>_Quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_ColorSpace:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Mjpeg Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>kerneldeint</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/kernelDeint/ADM_vidKernelDeint.cpp" line="+71"/>
        <source>_Threshold:</source>
        <translation type="unfinished">_Seuil:</translation>
    </message>
    <message>
        <location line="-21"/>
        <source>Kernel Deint.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Port of Donald Graft Kernel Deinterlacer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Field order:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Smaller means more deinterlacing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Sharp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>_Sharper engine:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>T_woway</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Extrapolate better (better not to use it)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Show interlaced areas (for test!)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>KernelDeint</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>lame</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/lame/audioencoder_lame.cpp" line="+333"/>
        <source>_Quality:</source>
        <translation type="unfinished">_Qualité:</translation>
    </message>
    <message>
        <location line="-23"/>
        <source>CBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>ABR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Bit_rate mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+18"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Disable reservoir:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>LAME Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>largemedian</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/largeMedian/ADM_largeMedian.cpp" line="+36"/>
        <source>Large Median (5x5).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Median filter on 5x5 matrix.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>_Process luma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Process luma plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>P_rocess chroma</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Fast Convolution</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>lavDeint</name>
    <message>
        <source>None</source>
        <translation type="obsolete">Aucun</translation>
    </message>
    <message>
        <source>Linear blend</source>
        <translation type="obsolete">Mélange linéaire</translation>
    </message>
    <message>
        <source>Linear interpolate</source>
        <translation type="obsolete">Interpolation linéaire</translation>
    </message>
    <message>
        <source>Cubic interpolate</source>
        <translation type="obsolete">Interpolation Cubique</translation>
    </message>
    <message>
        <source>Median interpolate</source>
        <translation type="obsolete">Interpolation Médiane</translation>
    </message>
    <message>
        <source>FFmpeg deint</source>
        <translation type="obsolete">FFmpeg</translation>
    </message>
    <message>
        <source>_Deinterlacing:</source>
        <translation type="obsolete">_Dé-entrelacement</translation>
    </message>
    <message>
        <source>_Autolevel</source>
        <translation type="obsolete">Niveau automatique de couleur</translation>
    </message>
    <message>
        <source>libavcodec deinterlacer</source>
        <translation type="obsolete">De-entralacement par Libavcodec</translation>
    </message>
</context>
<context>
    <name>lavcodec</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/lavcodec/audioencoder_lavcodec.cpp" line="+524"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>lavdecoder</name>
    <message>
        <location filename="../../../avidemux_core/ADM_coreVideoCodec/src/DIA_lavDecoder.cpp" line="+23"/>
        <source>_Swap U and V</source>
        <translation type="unfinished">Inverser U et V</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show motion _vectors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Decoder Options</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>lavdeint</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/lavDeint/lavDeint.cpp" line="+188"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Linear blend</source>
        <translation type="unfinished">Mélange linéaire</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Linear interpolate</source>
        <translation type="unfinished">Interpolation linéaire</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Cubic interpolate</source>
        <translation type="unfinished">Interpolation Cubique</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Median interpolate</source>
        <translation type="unfinished">Interpolation Médiane</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>FFmpeg deint</source>
        <translation type="unfinished">FFmpeg</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Deinterlacing:</source>
        <translation type="unfinished">_Dé-entrelacement</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Autolevel</source>
        <translation type="unfinished">Niveau automatique de couleur</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>libavcodec deinterlacer</source>
        <translation type="unfinished">De-entralacement par Libavcodec</translation>
    </message>
    <message>
        <location line="-131"/>
        <source>Libavdec Deinterlacers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lavcodec deinterlacer family.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>licenseDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/license.ui" line="+16"/>
        <source>License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+46"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>logo</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/logo/ADM_vidLogo.cpp" line="+22"/>
        <source>Add logo</source>
        <translation type="unfinished">Ajouter un logo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Put a logo on top of video, with alpha blending.</source>
        <translation type="unfinished">Ajouter un logo sur la vidéo, avec transparence</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/logo/qt4/Q_logo.cpp" line="+113"/>
        <source>Select Logo Image</source>
        <translation type="unfinished">Choisir l&apos;image</translation>
    </message>
</context>
<context>
    <name>logoDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/logo/qt4/logo.ui" line="+14"/>
        <source>Logo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Select</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Alpha will be ignored if the image has alpha channel (png in RGB)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Alpha</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>You can click in the image to set the logo approximatively</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>lumaonly</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/lumaOnly/lumaOnly.cpp" line="+44"/>
        <source>GreyScale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove color, only key grey image.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mainFilterDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_filters/mainfilter.ui" line="+20"/>
        <source>Video Filter Manager</source>
        <translation type="unfinished">Gestionnaire de filtres videos</translation>
    </message>
    <message>
        <location line="+61"/>
        <source>&lt;big&gt;&lt;b&gt;Active Filters&lt;/b&gt;&lt;/big&gt;</source>
        <translation type="unfinished">&lt;big&gt;&lt;b&gt;Filtres Actifs&lt;/b&gt;&lt;/big&gt;</translation>
    </message>
    <message>
        <source>C&amp;onfigure</source>
        <translation type="obsolete">C&amp;onfigurer</translation>
    </message>
    <message>
        <source>P&amp;artial</source>
        <translation type="obsolete">P&amp;artiel</translation>
    </message>
    <message>
        <source>Down</source>
        <translation type="obsolete">Plus bas</translation>
    </message>
    <message>
        <source>Up</source>
        <translation type="obsolete">Plus haut</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation type="obsolete">Enlever</translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="obsolete">Ajouter</translation>
    </message>
    <message>
        <location line="+78"/>
        <source>&lt;big&gt;&lt;b&gt;Available Filters&lt;/b&gt;&lt;/big&gt;</source>
        <translation type="unfinished">&lt;big&gt;&lt;b&gt;Filtres disponibles&lt;/b&gt;&lt;/big&gt;</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Transform</source>
        <translation type="unfinished">Transformation</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Interlacing</source>
        <translation type="unfinished">Entrelacement</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Colors</source>
        <translation type="unfinished">Couleurs</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Noise</source>
        <translation type="unfinished">Bruit</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Sharpness</source>
        <translation type="unfinished">Netteté</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Subtitles</source>
        <translation type="unfinished">Sous titre</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Miscellaneous</source>
        <translation type="unfinished">Divers</translation>
    </message>
    <message>
        <location line="+51"/>
        <source>&amp;Preview</source>
        <translation type="unfinished">&amp;Prévisualisation</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Close</source>
        <translation type="unfinished">&amp;Fermer</translation>
    </message>
    <message>
        <location line="-71"/>
        <source>OpenGl</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mainMenu</name>
    <message>
        <source>Open</source>
        <translation type="obsolete">Ouvrir</translation>
    </message>
    <message>
        <source>Append</source>
        <translation type="obsolete">Ajouter</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="obsolete">Sauver</translation>
    </message>
    <message>
        <source>Save as Image</source>
        <translation type="obsolete">Sauver comme image</translation>
    </message>
    <message>
        <source>Save as BMP</source>
        <translation type="obsolete">Sauver comme BMP</translation>
    </message>
    <message>
        <source>Save as JPEG</source>
        <translation type="obsolete">Sauver comme JPEG</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">Fermer</translation>
    </message>
    <message>
        <source>Connect to avsproxy</source>
        <translation type="obsolete">Se connecter à avsproxy</translation>
    </message>
    <message>
        <source>Quit</source>
        <translation type="obsolete">Quitter</translation>
    </message>
    <message>
        <source>Undo</source>
        <translation type="obsolete">Défaire</translation>
    </message>
    <message>
        <source>Reset Edit</source>
        <translation type="obsolete">Annuler toutes les éditions</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="obsolete">Couper</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="obsolete">Copier</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="obsolete">Coller</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Effacer</translation>
    </message>
    <message>
        <source>Set Marker A</source>
        <translation type="obsolete">Mettre le marqueur A</translation>
    </message>
    <message>
        <source>Set Marker B</source>
        <translation type="obsolete">Mettre le marqueur B</translation>
    </message>
    <message>
        <source>Decoder Option</source>
        <translation type="obsolete">Option du décodeur</translation>
    </message>
    <message>
        <source>Filters</source>
        <translation type="obsolete">Filtres</translation>
    </message>
    <message>
        <source>Select Track</source>
        <translation type="obsolete">Choisir les pistes</translation>
    </message>
    <message>
        <source>Save audio</source>
        <translation type="obsolete">Sauver la piste audio</translation>
    </message>
    <message>
        <source>Open Application &amp;Log</source>
        <translation type="obsolete">Ouvrir le fichier de log</translation>
    </message>
    <message>
        <source>Open Application Data &amp;Folder</source>
        <translation type="obsolete">Ouvrir le dossier de l&apos;application</translation>
    </message>
    <message>
        <source>About</source>
        <translation type="obsolete">A propos</translation>
    </message>
    <message>
        <source>Play/Stop</source>
        <translation type="obsolete">Jouer/stopper</translation>
    </message>
    <message>
        <source>Previous Frame</source>
        <translation type="obsolete">Image précédente</translation>
    </message>
    <message>
        <source>Next Frame</source>
        <translation type="obsolete">Image suivante</translation>
    </message>
    <message>
        <source>Previous Intra Frame</source>
        <translation type="obsolete">Image clé précédente</translation>
    </message>
    <message>
        <source>Next Intra Frame</source>
        <translation type="obsolete">Image clé suivante</translation>
    </message>
    <message>
        <source>Previous Black Frame</source>
        <translation type="obsolete">Image noire précédente</translation>
    </message>
    <message>
        <source>Next Black Frame</source>
        <translation type="obsolete">Image noire suivante</translation>
    </message>
    <message>
        <source>First Frame</source>
        <translation type="obsolete">Première image</translation>
    </message>
    <message>
        <source>Last Frame</source>
        <translation type="obsolete">Dernière image</translation>
    </message>
    <message>
        <source>Go To Marker A</source>
        <translation type="obsolete">Aller au marqueur A</translation>
    </message>
    <message>
        <source>Go To Marker B</source>
        <translation type="obsolete">Aller au marqueur B</translation>
    </message>
    <message>
        <source>Go To Time</source>
        <translation type="obsolete">Aller au temps</translation>
    </message>
</context>
<context>
    <name>matroskademuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Matroska/ADM_mkvIndexer.cpp" line="+48"/>
        <source>Matroska Images</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+425"/>
        <source>Matroska clusters</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mcdeint</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mcDeint/ADM_vidMcDeint.cpp" line="+81"/>
        <source>MCDeint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Motion compensation deinterlacer. Ported from MPlayer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Fast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Medium</source>
        <translation type="unfinished">Moyenne</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Slow iterative motion search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Extra slow (same as 3+multiple reference frames)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom :</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Qp:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>mcDeinterlace</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mean</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fastConvolution/Mean.cpp" line="+27"/>
        <source>Mean convolution.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>3x3 convolution filter :mean.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>median</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fastConvolution/Median.cpp" line="+28"/>
        <source>Median convolution.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>3x3 convolution filter :median.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mergeFields</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fields/ADM_vidMergeField.cpp" line="+52"/>
        <source>Merge Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Merge two pictures as if they were two fields.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mkvmuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMkv/muxerMkv.cpp" line="+134"/>
        <source>Saving Mkv</source>
        <translation type="unfinished">Sauvegarde au format MKV (ff)</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMkv/muxerMkvConfig.cpp" line="+27"/>
        <source>Force display width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>MKV Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mp3d</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mplayerDenoise3D/ADM_vidMPLD3D.cpp" line="+235"/>
        <source>_Spatial luma strength:</source>
        <translation type="unfinished">Luma, intensité spatiale:</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+1"/>
        <source>Luma _Temporal strength:</source>
        <translation type="unfinished">Luma, intensité temporelle:</translation>
    </message>
    <message>
        <location line="-37"/>
        <source>Mplayer Denoise 3D HQ</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Reduce noise, smooth image, increase compressibility. HQ Version.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>S_patial chroma strength:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>MPlayer denoise3d</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mp3dlow</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mplayerDenoise3D/ADM_vidMPLD3Dlow.cpp" line="+69"/>
        <source>_Spatial luma strength:</source>
        <translation type="unfinished">Luma, intensité spatiale:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Temporal strength:</source>
        <translation type="unfinished">Intensité temporelle:</translation>
    </message>
    <message>
        <location line="-35"/>
        <source>Mplayer Denoise 3D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Reduce noise, smooth image, increase compressibility.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>S_patial chroma strength:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>MPlayer denoise3d</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mp4demuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Mp4/ADM_mp4Analyzer.cpp" line="+791"/>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Mp4/ADM_mp4Analyzer.cpp.orig" line="+735"/>
        <source>Problem reading SVQ3 headers</source>
        <translation type="unfinished">Impossible de lire l&apos;entete SVQ3</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/Mp4/ADM_mp4Indexer.cpp.orig" line="+343"/>
        <source>No stts table</source>
        <translation type="unfinished">Pas d&apos;element STTS</translation>
    </message>
</context>
<context>
    <name>mp4muxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMp4/muxerMP4.cpp" line="+64"/>
        <location line="+9"/>
        <source>Unsupported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-9"/>
        <source>Only MP4Video, H264, and H265 supported for video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Only AAC, AC3, and mpegaudio supported for audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Saving mp4</source>
        <translation type="unfinished">Sauvegarde au format MP4</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Saving PSP</source>
        <translation type="unfinished">Sauvegarde au format PSP</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMp4/muxerMP4Config.cpp" line="+27"/>
        <source>Muxing Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Use alternate MP3 tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>MP4 Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mp4v2muxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMp4v2/muxerMp4v2.cpp" line="+180"/>
        <source>Audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Audio format not supported, only AAC/MP3/AC3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Saving</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Video does not have enough timing information. Are you copying from AVI?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+65"/>
        <source>Cannot rename file (optimize)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMp4v2/muxerMp4v2Audio.cpp" line="+53"/>
        <source>Invalid frequency for AC3. Only 32, 44.1 &amp; 48 kHz</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Invalid bitrate for AC3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Invalid number of channels for AC3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+48"/>
        <source>Cannot get AAC Extra data
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerMp4v2/muxerMp4v2Config.cpp" line="+31"/>
        <source>Optimize for streaming (SLOW)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add ipod metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>MP4V2 Settings</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>mpdelogoDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mplayerDelogo/qt4/mpdelogo.ui" line="+14"/>
        <source>Mplayer Delogo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Border Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Preview. Click in the image above to coarsly set the box coordinates</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>msharpen</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/mSharpen/ADM_vidMSharpen.cpp" line="+136"/>
        <source>_Threshold:</source>
        <translation type="unfinished">_Seuil:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Strength:</source>
        <translation type="unfinished">Force:</translation>
    </message>
    <message>
        <location line="-60"/>
        <source>Msharpen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Sharpen edges without amplifying noise. By Donald Graft.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+55"/>
        <source>_Mask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_High Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>MSharpen</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>muxerMP4</name>
    <message>
        <source>Saving mp4</source>
        <translation type="obsolete">Sauvegarde au format MP4</translation>
    </message>
    <message>
        <source>Saving PSP</source>
        <translation type="obsolete">Sauvegarde au format PSP</translation>
    </message>
</context>
<context>
    <name>muxerMkv</name>
    <message>
        <source>Saving Mkv</source>
        <translation type="obsolete">Sauvegarde au format MKV (ff)</translation>
    </message>
</context>
<context>
    <name>muxerffPS</name>
    <message>
        <source>Saving mpeg PS (ff)</source>
        <translation type="obsolete">Sauvegarde au format PS (ff)</translation>
    </message>
</context>
<context>
    <name>muxerffTS</name>
    <message>
        <source>Saving mpeg TS (ff)</source>
        <translation type="obsolete">Sauvegarde au format TS (ff)</translation>
    </message>
</context>
<context>
    <name>navigate</name>
    <message>
        <location filename="../../common/gui_navigate.cpp" line="+227"/>
        <source>Cannot go to next keyframe</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Cannot go to previous keyframe</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+176"/>
        <source>Seek</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
</context>
<context>
    <name>nvenc</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/nvEnc/ADM_nvEnc.cpp" line="+136"/>
        <source>Low Quality</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>High Quality</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>BluRay</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency (LQ)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Low Latency (HQ)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Preset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bitrate (kbps):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Max Bitrate (kbps):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Nvidia H264 Encoder configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>openGlResize</name>
    <message>
        <source>Width :</source>
        <translation type="obsolete">Largeur:</translation>
    </message>
    <message>
        <source>Height :</source>
        <translation type="obsolete">Hauteur:</translation>
    </message>
</context>
<context>
    <name>opendmldemuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/OpenDml/ADM_openDML.cpp" line="+321"/>
        <location line="+76"/>
        <location line="+264"/>
        <source>Malformed header</source>
        <translation type="unfinished">En tete incorrecte</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/OpenDml/ADM_openDMLDepack.cpp" line="+89"/>
        <source>Unpacking bitstream</source>
        <translation type="unfinished">Conversion en cours</translation>
    </message>
</context>
<context>
    <name>partial</name>
    <message>
        <location filename="../../common/ADM_videoFilter2/src/ADM_vidPartial.cpp" line="+299"/>
        <source>Start time (ms):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>End time (ms):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Configure filter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Partial Filter</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>printinfo</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/printInfo/printInfo.cpp" line="+44"/>
        <source>PrintInfo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display some informations on Screen.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>propsDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/props.ui" line="+14"/>
        <source>Properties</source>
        <translation type="unfinished">Propriétés</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Frame Rate:</source>
        <translation type="unfinished">Images/s:</translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+117"/>
        <source>Total Duration:</source>
        <translation type="unfinished">Durée totale:</translation>
    </message>
    <message>
        <location line="-103"/>
        <source>Aspect Ratio:</source>
        <translation type="unfinished">Rapport h/l:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Image Size:</source>
        <translation type="unfinished">Dimensions:</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Extra Video Properties</source>
        <translation type="unfinished">Informations supp</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>ExtraDataSize:</source>
        <translation type="unfinished">Taille:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Extra data :</source>
        <translation type="unfinished">Infos:</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Frequency:</source>
        <translation type="unfinished">Fréquence:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Variable Bitrate:</source>
        <translation type="unfinished">Bitrate variable:</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Channels:</source>
        <translation type="unfinished">Canaux:</translation>
    </message>
    <message>
        <location line="-209"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+14"/>
        <location line="+14"/>
        <location line="+14"/>
        <location line="+14"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Codec 4CC:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Codec:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>psdemuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegPS/ADM_ps.cpp" line="+61"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This file&apos;s index has been created with an older version of avidemux.
Please delete the idx2 file and reopen.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegPS/ADM_psIndex.cpp" line="+159"/>
        <source>Indexing</source>
        <translation type="unfinished">Indexation</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>There are several files with sequential file names. Should they be all loaded ?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qaudiotracks</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_audioTracks.cpp" line="+134"/>
        <source>Select audio file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+140"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="-140"/>
        <source>Cannot use that file as audio track</source>
        <translation type="unfinished">Impossible d&apos;utiliser ce fichier comme piste audio</translation>
    </message>
    <message>
        <location line="+140"/>
        <source>Some tracks are used multiple times</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+126"/>
        <source>Track </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source> from video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>File </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>.... Add audio track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>copy</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qencoding</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_encoding.cpp" line="+57"/>
        <location line="+20"/>
        <source>Privileges Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-20"/>
        <location line="+20"/>
        <source>Root privileges are required to perform this operation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+284"/>
        <source>The encoding is paused. Do you want to resume or abort?</source>
        <translation type="unfinished">L&apos;encodage est en pause. Voulez vous reprendre ou abandonner ?</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Resume</source>
        <translation type="unfinished">Reprendre</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Abort</source>
        <translation type="unfinished">Abandonner</translation>
    </message>
</context>
<context>
    <name>qgui2</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/Q_gui2.cpp" line="+218"/>
        <source>&lt;b&gt;New version available&lt;/b&gt;&lt;br&gt; Version %1&lt;br&gt;Released on %2.&lt;br&gt;You can download it here&lt;br&gt; &lt;a href=&apos;%3&apos;&gt;%3&lt;/a&gt;&lt;br&gt;&lt;br&gt;&lt;small&gt; You can disable autoupdate in preferences.&lt;/small&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+212"/>
        <source>Recent Files</source>
        <translation type="unfinished">Fichiers Récents</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Recent Projects</source>
        <translation type="unfinished">Projets Récents</translation>
    </message>
    <message>
        <location line="+575"/>
        <source>The application has encountered a fatal problem
The current editing has been saved and will be reloaded at next start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+256"/>
        <source>%c-%s (%02d)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+257"/>
        <source> (%d track(s))</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qgui2menu</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/Q_gui2_menu.cpp" line="+51"/>
        <source>Project Script</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source> Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&amp;Run Project...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Debug Project...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save &amp;As Project...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Scripting Shell</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source> Shell</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Scripting Reference</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source> Reference</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qjobs</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_jobs.cpp" line="+13"/>
        <source>Ready</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Succeeded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Deleted</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Running</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Job Name</source>
        <translation type="unfinished">Nom du job</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Start Time</source>
        <translation type="unfinished">Date de départ</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>End Time</source>
        <translation type="unfinished">Date de fin</translation>
    </message>
    <message>
        <location line="+64"/>
        <location line="+14"/>
        <source>Sure!</source>
        <translation type="unfinished">Sur!</translation>
    </message>
    <message>
        <location line="-14"/>
        <source>Delete job</source>
        <translation type="unfinished">Détruire le job</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Are you sure you want to delete %s job?</source>
        <translation type="unfinished">Etes vous sur de vouloir détruire %s job ?</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Delete *all* job</source>
        <translation type="unfinished">Detruire *tous* les jobs</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Are you sure you want to delete ALL jobs?</source>
        <translation type="unfinished">Etes vous sur de vouloir détruire TOUS les jobs ?</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Already done</source>
        <translation type="unfinished">Déjà fait</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This script has already been successfully executed.</source>
        <translation type="unfinished">Ce script a déjà été executé avec succés</translation>
    </message>
</context>
<context>
    <name>qlicense</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_license.cpp" line="+25"/>
        <source>&lt;!DOCTYPE html PUBLIC &quot;-//W3C//DTD HTML 4.01 Transitional//EN&quot;&gt;&lt;html&gt;&lt;head&gt;  &lt;title&gt;Avidemux is free software; you can redistribute it and/or  modify it under the terms of the GNU General Public License  version 2 as published by the Free Software Foundation&lt;/title&gt;&lt;style type=&quot;text/css&quot;&gt;&lt;!-- /* Style Definitions */ p.licenseStyle, li.licenseStyle, div.licenseStyle        {margin:0cm;        margin-bottom:.0001pt;        font-size:12.0pt;        font-family:&quot;Times New Roman&quot;;} /* Page Definitions */ @page Section1        {size:612.0pt 792.0pt;        margin:72.0pt 90.0pt 72.0pt 90.0pt;}div.Section1        {page:Section1;}--&gt;&lt;/style&gt;&lt;/head&gt;&lt;body lang=&quot;EN-GB&quot; style=&apos;text-justify-trim:punctuation&apos;&gt;  &lt;div class=&quot;Section1&quot;&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt;Avidemux is    free software; you can redistribute it and/or modify it under    the terms of the GNU General Public License version 2 as    published by the Free Software Foundation.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;b&gt;&lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt;    &lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;GNU GENERAL PUBLIC    LICENSE&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Version 2, June    1991&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Copyright (C) 1989, 1991    Free Software Foundation, Inc.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span lang=&quot;FR&quot; style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;59 Temple Place, Suite    330, Boston, MA  02111-1307  USA&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;span lang=&quot;FR&quot; style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Everyone is permitted to    copy and distribute verbatim copies of this license document,    but changing it is not allowed.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Preamble&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The licenses for most    software are designed to take away your freedom to share and    change it.  By contrast, the GNU General Public License is    intended to guarantee your freedom to share and change free    software--to make sure the software is free for all its users.     This General Public License applies to most of the Free    Software Foundation&apos;s software and to any other program whose    authors commit to using it.  (Some other Free Software    Foundation software is covered by the GNU Library General    Public License instead.)  You can apply it to your programs,    too.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;When we speak of free    software, we are referring to freedom, not price.  Our General    Public Licenses are designed to make sure that you have the    freedom to distribute copies of free software (and charge for    this service if you wish), that you receive source code or can    get it if you want it, that you can change the software or use    pieces of it in new free programs; and that you know you can do    these things.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;To protect your rights, we    need to make restrictions that forbid anyone to deny you these    rights or to ask you to surrender the rights. These    restrictions translate to certain responsibilities for you if    you distribute copies of the software, or if you modify    it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;For example, if you    distribute copies of such a program, whether gratis or for a    fee, you must give the recipients all the rights that you have.     You must make sure that they, too, receive or can get the    source code.  And you must show them these terms so they know    their rights.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;We protect your rights    with two steps: (1) copyright the software, and (2) offer you    this license which gives you legal permission to copy,    distribute and/or modify the software.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Also, for each author&apos;s    protection and ours, we want to make certain that everyone    understands that there is no warranty for this free software.     If the software is modified by someone else and passed on, we    want its recipients to know that what they have is not the    original, so that any problems introduced by others will not    reflect on the original authors&apos; reputations.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Finally, any free program    is threatened constantly by software patents.  We wish to avoid    the danger that redistributors of a free program will    individually obtain patent licenses, in effect making the    program proprietary.  To prevent this, we have made it clear    that any patent must be licensed for everyone&apos;s free use or not    licensed at all.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The precise terms and    conditions for copying, distribution and modification    follow.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;GNU GENERAL PUBLIC    LICENSE&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;TERMS AND CONDITIONS FOR    COPYING, DISTRIBUTION AND MODIFICATION&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;0. This License applies to    any program or other work which contains a notice placed by the    copyright holder saying it may be distributed under the terms    of this General Public License.  The &quot;Program&quot;, below, refers    to any such program or work, and a &quot;work based on the Program&quot;    means either the Program or any derivative work under copyright    law: that is to say, a work containing the Program or a portion    of it, either verbatim or with modifications and/or translated    into another language.  (Hereinafter, translation is included    without limitation in the term &quot;modification&quot;.)  Each licensee    is addressed as &quot;you&quot;.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Activities other than    copying, distribution and modification are not covered by this    License; they are outside its scope.  The act of running the    Program is not restricted, and the output from the Program is    covered only if its contents constitute a work based on the    Program (independent of having been made by running the    Program). Whether that is true depends on what the Program    does.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;1. You may copy and    distribute verbatim copies of the Program&apos;s source code as you    receive it, in any medium, provided that you conspicuously and    appropriately publish on each copy an appropriate copyright    notice and disclaimer of warranty; keep intact all the notices    that refer to this License and to the absence of any warranty;    and give any other recipients of the Program a copy of this    License along with the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;You may charge a fee for    the physical act of transferring a copy, and you may at your    option offer warranty protection in exchange for a    fee.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;2. You may modify your    copy or copies of the Program or any portion of it, thus    forming a work based on the Program, and copy and distribute    such modifications or work under the terms of Section 1 above,    provided that you also meet all of these conditions:&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;a) You must cause the    modified files to carry prominent notices stating that you    changed the files and the date of any change.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;b) You must cause any work    that you distribute or publish, that in whole or in part    contains or is derived from the Program or any part thereof, to    be licensed as a whole at no charge to all third parties under    the terms of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;c) If the modified program    normally reads commands interactively when run, you must cause    it, when started running for such interactive use in the most    ordinary way, to print or display an announcement including an    appropriate copyright notice and a notice that there is no    warranty (or else, saying that you provide a warranty) and that    users may redistribute the program under these conditions, and    telling the user how to view a copy of this License.     (Exception: if the Program itself is interactive but does not    normally print such an announcement, your work based on the    Program is not required to print an announcement.)&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;These requirements apply    to the modified work as a whole.  If identifiable sections of    that work are not derived from the Program, and can be    reasonably considered independent and separate works in    themselves, then this License, and its terms, do not apply to    those sections when you distribute them as separate works.  But    when you distribute the same sections as part of a whole which    is a work based on the Program, the distribution of the whole    must be on the terms of this License, whose permissions for    other licensees extend to the entire whole, and thus to each    and every part regardless of who wrote it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Thus, it is not the intent    of this section to claim rights or contest your rights to work    written entirely by you; rather, the intent is to exercise the    right to control the distribution of derivative or collective    works based on the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;In addition, mere    aggregation of another work not based on the Program with the    Program (or with a work based on the Program) on a volume of a    storage or distribution medium does not bring the other work    under the scope of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;3. You may copy and    distribute the Program (or a work based on it, under Section 2)    in object code or executable form under the terms of Sections 1    and 2 above provided that you also do one of the    following:&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;a) Accompany it with the    complete corresponding machine-readable source code, which must    be distributed under the terms of Sections 1 and 2 above on a    medium customarily used for software interchange;    or,&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;b) Accompany it with a    written offer, valid for at least three years, to give any    third party, for a charge no more than your cost of physically    performing source distribution, a complete machine-readable    copy of the corresponding source code, to be distributed under    the terms of Sections 1 and 2 above on a medium customarily    used for software interchange; or,&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;c) Accompany it with the    information you received as to the offer to distribute    corresponding source code.  (This alternative is allowed only    for noncommercial distribution and only if you received the    program in object code or executable form with such an offer,    in accord with Subsection b above.)&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;The source code for a work    means the preferred form of the work for making modifications    to it.  For an executable work, complete source code means all    the source code for all modules it contains, plus any    associated interface definition files, plus the scripts used to    control compilation and installation of the executable.     However, as a special exception, the source code distributed    need not include anything that is normally distributed (in    either source or binary form) with the major components    (compiler, kernel, and so on) of the operating system on which    the executable runs, unless that component itself accompanies    the executable.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;If distribution of    executable or object code is made by offering access to copy    from a designated place, then offering equivalent access to    copy the source code from the same place counts as distribution    of the source code, even though third parties are not compelled    to copy the source along with the object code.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;4. You may not copy,    modify, sublicense, or distribute the Program except as    expressly provided under this License.  Any attempt otherwise    to copy, modify, sublicense or distribute the Program is void,    and will automatically terminate your rights under this    License. However, parties who have received copies, or rights,    from you under this License will not have their licenses    terminated so long as such parties remain in full    compliance.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;5. You are not required to    accept this License, since you have not signed it.  However,    nothing else grants you permission to modify or distribute the    Program or its derivative works.  These actions are prohibited    by law if you do not accept this License.  Therefore, by    modifying or distributing the Program (or any work based on the    Program), you indicate your acceptance of this License to do    so, and all its terms and conditions for copying, distributing    or modifying the Program or works based on it.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;6. Each time you    redistribute the Program (or any work based on the Program),    the recipient automatically receives a license from the    original licensor to copy, distribute or modify the Program    subject to these terms and conditions.  You may not impose any    further restrictions on the recipients&apos; exercise of the rights    granted herein. You are not responsible for enforcing    compliance by third parties to this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;7. If, as a consequence of    a court judgment or allegation of patent infringement or for    any other reason (not limited to patent issues), conditions are    imposed on you (whether by court order, agreement or otherwise)    that contradict the conditions of this License, they do not    excuse you from the conditions of this License.  If you cannot    distribute so as to satisfy simultaneously your obligations    under this License and any other pertinent obligations, then as    a consequence you may not distribute the Program at all.  For    example, if a patent license would not permit royalty-free    redistribution of the Program by all those who receive copies    directly or indirectly through you, then the only way you could    satisfy both it and this License would be to refrain entirely    from distribution of the Program.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;If any portion of this    section is held invalid or unenforceable under any particular    circumstance, the balance of the section is intended to apply    and the section as a whole is intended to apply in other    circumstances.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;It is not the purpose of    this section to induce you to infringe any patents or other    property right claims or to contest validity of any such    claims; this section has the sole purpose of protecting the    integrity of the free software distribution system, which is    implemented by public license practices.  Many people have made    generous contributions to the wide range of software    distributed through that system in reliance on consistent    application of that system; it is up to the author/donor to    decide if he or she is willing to distribute software through    any other system and a licensee cannot impose that    choice.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;This section is intended    to make thoroughly clear what is believed to be a consequence    of the rest of this License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;8. If the distribution    and/or use of the Program is restricted in certain countries    either by patents or by copyrighted interfaces, the original    copyright holder who places the Program under this License may    add an explicit geographical distribution limitation excluding    those countries, so that distribution is permitted only in or    among countries not thus excluded.  In such case, this License    incorporates the limitation as if written in the body of this    License.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;9. The Free Software    Foundation may publish revised and/or new versions of the    General Public License from time to time.  Such new versions    will be similar in spirit to the present version, but may    differ in detail to address new problems or    concerns.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;Each version is given a    distinguishing version number.  If the Program specifies a    version number of this License which applies to it and &quot;any    later version&quot;, you have the option of following the terms and    conditions either of that version or of any later version    published by the Free Software Foundation.  If the Program does    not specify a version number of this License, you may choose    any version ever published by the Free Software    Foundation.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;10. If you wish to    incorporate parts of the Program into other free programs whose    distribution conditions are different, write to the author to    ask for permission.  For software which is copyrighted by the    Free Software Foundation, write to the Free Software    Foundation; we sometimes make exceptions for this.  Our    decision will be guided by the two goals of preserving the free    status of all derivatives of our free software and of promoting    the sharing and reuse of software generally.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;NO WARRANTY&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;11. BECAUSE THE PROGRAM IS    LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM,    TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN    OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER    PARTIES PROVIDE THE PROGRAM &quot;AS IS&quot; WITHOUT WARRANTY OF ANY    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A    PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND    PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM    PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY    SERVICING, REPAIR OR CORRECTION.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=    &apos;text-align:justify;text-autospace:none&apos;&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;12. IN NO EVENT UNLESS    REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY    COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR    REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU    FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR    CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO    USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR    DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR    THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY    OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.&lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; align=&quot;center&quot; style=    &apos;text-align:center;text-autospace:none&apos;&gt;&lt;b&gt;&lt;span style=    &apos;font-size:10.0pt;font-family:Arial&apos;&gt;END OF TERMS AND    CONDITIONS&lt;/span&gt;&lt;/b&gt;&lt;/p&gt;    &lt;p class=&quot;licenseStyle&quot; style=&apos;text-autospace:none&apos;&gt;    &lt;span style=&apos;font-size:10.0pt; font-family:Arial&apos;&gt; &lt;/span&gt;&lt;/p&gt;  &lt;/div&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qmainfilter</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_filters/Q_mainfilter.cpp" line="+318"/>
        <source>Partial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This filter cannot be made partial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+177"/>
        <source>Add</source>
        <translation type="unfinished">Ajouter</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Remove</source>
        <translation type="unfinished">Enlever</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Configure</source>
        <translation type="unfinished">Configurer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Move up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Move down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Make partial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Load video filters..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Save video filters..</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qprocessing</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_processing.cpp" line="+66"/>
        <source>Unknown</source>
        <translation type="unfinished">Inconnu</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>_Resume</source>
        <translation type="unfinished">_Reprendre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The processing is paused.</source>
        <translation type="unfinished">L&apos;operation est en pause</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Cancel it ?</source>
        <translation type="unfinished">Annuler ?</translation>
    </message>
</context>
<context>
    <name>qprops</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_props.cpp" line="+23"/>
        <source>No</source>
        <translation type="unfinished">Non</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Yes</source>
        <translation type="unfinished">Oui</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>%2.3f fps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <location line="+59"/>
        <source>%02d:%02d:%02d.%03d</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-53"/>
        <source>%s (%u:%u)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Mono</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Stereo</source>
        <translation type="unfinished">Stéréo</translation>
    </message>
</context>
<context>
    <name>qshell</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_shell/Q_shell.cpp" line="+43"/>
        <location filename="../ADM_userInterfaces/ADM_shell/Q_shell.cpp.rej" line="+9"/>
        <source>Enter your commands then press the evaluate button or CTRL+ENTER.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../ADM_userInterfaces/ADM_shell/Q_shell.cpp.rej" line="+1"/>
        <source>You can use CTRL+PageUP and CTRL+Page Down to recall previous commands
Ready.
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qtalert</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/alert_qt4.cpp" line="+32"/>
        <source>Alert</source>
        <translation type="unfinished">Alerte</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+25"/>
        <location line="+25"/>
        <source>Info</source>
        <translation type="unfinished">Information</translation>
    </message>
    <message>
        <location line="+24"/>
        <location line="+30"/>
        <source>Confirmation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <location line="+30"/>
        <source>Question</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-7"/>
        <source>Question?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>qtray</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_gui/ADM_qtray.cpp" line="+82"/>
        <location filename="../ADM_userInterfaces/ADM_gui/ADM_qtray.cpp.rej" line="+1"/>
        <source>Open Avidemux</source>
        <translation type="unfinished">Ouvrir Avidemux</translation>
    </message>
</context>
<context>
    <name>qvobsub</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/Q_vobsub.cpp" line="+88"/>
        <source>Select Idx File</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>removeplane</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/removePlane/removePlane.cpp" line="+45"/>
        <source>Remove  Plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Y,U or V plane (used mainly to debug other filters).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+105"/>
        <source>Keep Y Plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Process luma plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Keep U Plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Process chromaU plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Keep V Plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Process chromaV plane</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove plane</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>resampleFps</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/resampleFps/ADM_vidResampleFPS.cpp" line="+43"/>
        <source>Custom</source>
        <translation type="unfinished">Manuel</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Resample FPS</source>
        <translation type="unfinished">Re-échantillonner le nombre d&apos;I/S</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Change and enforce FPS. Keep duration and sync.</source>
        <translation type="unfinished">Change le nombre d&apos;i/s, conserve la synchronisation et la durée</translation>
    </message>
    <message>
        <location line="+275"/>
        <source>_New frame rate:</source>
        <translation type="unfinished">Nouveau nombre d&apos;i/s:</translation>
    </message>
    <message>
        <location line="-315"/>
        <source>25  (PAL)</source>
        <translation type="unfinished">25  (PAL/SECAM)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>23.976 (Film)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>29.97 (NTSC)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>50 (Pal)</source>
        <translation type="unfinished">50 (PAL/SECAM)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>59.93  (NTSC)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+310"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Resample fps</source>
        <translation type="unfinished">Re-échantillonnage du nombre d&apos;i/s</translation>
    </message>
</context>
<context>
    <name>resize</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/resize/qt4/Q_resize.cpp" line="+251"/>
        <source>Width and height cannot be odd</source>
        <translation type="unfinished">La largeur ou la hauteur ne peuvent être impaires</translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/resize/swScaleResize.cpp" line="+80"/>
        <source>swsResize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>swScale Resizer.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>resizeDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/resize/qt4/resizing.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+13"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+13"/>
        <source>Resize</source>
        <translation type="unfinished">Redimenssionner</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+12"/>
        <source>Aspect Ratio</source>
        <translation type="unfinished">Rapport hauteur/largeur</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+12"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+12"/>
        <source>Lock Aspect Ratio</source>
        <translation type="unfinished">Vérrouiller le rapport h/l</translation>
    </message>
    <message>
        <location line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+104"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+104"/>
        <source>Resize Dimensions</source>
        <translation type="unfinished">Redimensionner vers </translation>
    </message>
    <message>
        <location line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+20"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+20"/>
        <source>Width:</source>
        <translation type="unfinished">Largeur:</translation>
    </message>
    <message>
        <location line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+36"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+36"/>
        <source>Height:</source>
        <translation type="unfinished">Hauteur:</translation>
    </message>
    <message>
        <location line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+35"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+35"/>
        <source>Round to the Nearest Multiple of 16</source>
        <translation type="unfinished">Arrondir au multiple de 16</translation>
    </message>
    <message>
        <location line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+75"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+75"/>
        <source>Percent</source>
        <translation type="unfinished">Pourcent</translation>
    </message>
    <message>
        <location line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+70"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+70"/>
        <source>Error X / Y:</source>
        <translation type="unfinished">Erreur X/Y:</translation>
    </message>
    <message>
        <location line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+58"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+58"/>
        <source>Resize Method:</source>
        <translation type="unfinished">Méthode:</translation>
    </message>
    <message>
        <location line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+8"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+8"/>
        <source>Bilinear</source>
        <translation type="unfinished">Bilinéaire</translation>
    </message>
    <message>
        <location line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+5"/>
        <source>Bicubic</source>
        <translation type="unfinished">Bicuibique</translation>
    </message>
    <message>
        <location line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+5"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+5"/>
        <source>Lanzcos3</source>
        <translation type="unfinished">Lanzcos3</translation>
    </message>
    <message>
        <location line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="-398"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="-398"/>
        <source>Source:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+8"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+8"/>
        <location line="+42"/>
        <source>1:1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <source>4:3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="-37"/>
        <location line="+42"/>
        <source>16:9</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="-18"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="-18"/>
        <source>Destination:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+190"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+190"/>
        <source>1%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+40"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+40"/>
        <source>200%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glGlyphy/glGlyphy.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glResize/sampleGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glRotate/rotateGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glSmooth/glSmooth.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glVdpau/ADM_vf_vdpauGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/glYadif/glYadif.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_distort/sampleGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment/sampleGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_fragment2/sampleGl.ui" line="+57"/>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6_openGl/sample_vertex/sampleGl.ui" line="+57"/>
        <source>0.00 / 0.00</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>resizeWindow</name>
    <message>
        <source>Width and height cannot be odd</source>
        <translation type="obsolete">La largeur ou la hauteur ne peuvent être impaires</translation>
    </message>
</context>
<context>
    <name>rotate</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/rotate/rotate.cpp" line="+52"/>
        <location line="+163"/>
        <source>Rotate</source>
        <translation type="unfinished">Rotation</translation>
    </message>
    <message>
        <location line="-162"/>
        <source>Rotate the image by 90/180/270 degrees.</source>
        <translation type="unfinished">Angle de rotation (90/180/270);</translation>
    </message>
    <message>
        <location line="+155"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>90 degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>90°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>180 degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>180°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>270 degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>270°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Angle:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>rotateFilter</name>
    <message>
        <source>None</source>
        <translation type="obsolete">Aucun</translation>
    </message>
</context>
<context>
    <name>seekablePreviewDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_filters/seekablePreview.ui" line="+20"/>
        <source>Preview</source>
        <translation type="unfinished">Aperçu</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>next</source>
        <translation type="unfinished">Suivant</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>00:00:00.000</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>separateFields</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fields/ADM_vidSeparateField.cpp" line="+53"/>
        <source>Separate Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Split each image into 2 fields.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>sharpen</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/fastConvolution/Sharpen.cpp" line="+27"/>
        <source>Sharpen convolution.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>3x3 convolution filter :sharpen.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>stackfield</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/stackField/stackField.cpp" line="+47"/>
        <source>Stack Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Put even lines on top, odd lines at bottom.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>subAss</name>
    <message>
        <source>_Subtitle file (ASS/SSA):</source>
        <translation type="obsolete">Fichier de sous-titres (Ass/Ssa):</translation>
    </message>
    <message>
        <source>Select Subtitle file</source>
        <translation type="obsolete">Sélectionner sous titres</translation>
    </message>
    <message>
        <source>_Line spacing:</source>
        <translation type="obsolete">Inter-Lignes:</translation>
    </message>
    <message>
        <source>_Font scale:</source>
        <translation type="obsolete">Taille de la fonte:</translation>
    </message>
    <message>
        <source>_Top margin:</source>
        <translation type="obsolete">Marge haute:</translation>
    </message>
    <message>
        <source>Botto_m margin</source>
        <translation type="obsolete">Marge basse </translation>
    </message>
</context>
<context>
    <name>swapuv</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/swapUV/swapUV.cpp" line="+41"/>
        <source>Swap UV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Swap the U and V planes.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>telecide</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/telecide/Telecide.cpp" line="+45"/>
        <source>Decomb telecide</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Donald Graft Telecide. Replace ivtc pattern by progressive frames. Video stays at 30 fps.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/telecide/Telecide_utils.cpp" line="+185"/>
        <source>No strategy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>3:2 pulldown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PAL/SECAM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>NTSC converted from PAL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Never</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>If still combed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Always</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>None but compute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Postproc on best match</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Postproc and show zones (debug)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Process image (not fields)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Process image (not fields), debug</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Strategy:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Field order:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Postprocessing:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Try backward:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Direct threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Backward threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Noise threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Postp_rocessing threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Use chroma to decide</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Sho_w info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Debu_g</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bl_end</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Decomb Telecide</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>tsdemuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegTS/ADM_ts.cpp" line="+70"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>This file&apos;s index has been created with an older version of avidemux.
Please delete the idx2 file and reopen.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegTS/ADM_tsIndexH264.cpp" line="+124"/>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegTS/ADM_tsIndexMpeg2.cpp" line="+82"/>
        <source>There are several files with sequential file names. Should they be all loaded ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegTS/ADM_tsIndexMpeg2.cpp" line="+7"/>
        <location filename="../../../avidemux_plugins/ADM_demuxers/MpegTS/ADM_tsIndexVC1.cpp" line="+66"/>
        <source>Indexing</source>
        <translation type="unfinished">Indexation</translation>
    </message>
</context>
<context>
    <name>twolame</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/twolame/audioencoder_twolame.cpp" line="+205"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>TwoLame Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>uisupport</name>
    <message>
        <location filename="../ADM_userInterfaces/ui_support.cpp" line="+191"/>
        <source>X11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>MS Windows GDI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Mac OS X Quartz 2D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Mac OS X QuickDraw</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>OpenGL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>MS Windows Direct3D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Default Raster</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>unstackfield</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/stackField/unstackField.cpp" line="+47"/>
        <source>Unstack Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Weave top and bottom halves.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>vdpauVideoFilter</name>
    <message>
        <source>Width :</source>
        <translation type="obsolete">Largeur:</translation>
    </message>
    <message>
        <source>Height :</source>
        <translation type="obsolete">Hauteur:</translation>
    </message>
    <message>
        <source>vdpau</source>
        <translation type="obsolete">vdpau</translation>
    </message>
</context>
<context>
    <name>vdpauVideoFilterDeint</name>
    <message>
        <source>Keep Top Field</source>
        <translation type="obsolete">Garder le champs supérieur</translation>
    </message>
    <message>
        <source>Keep Bottom Field</source>
        <translation type="obsolete">Garder le champs inférieur</translation>
    </message>
    <message>
        <source>Double framerate</source>
        <translation type="obsolete">Doubler le nombre d&apos;image par seconde</translation>
    </message>
    <message>
        <source>_Resize:</source>
        <translation type="obsolete">_Redimensionner</translation>
    </message>
    <message>
        <source>_Deint Mode:</source>
        <translation type="obsolete">Mode de _deentrelacement:</translation>
    </message>
    <message>
        <source>Width :</source>
        <translation type="obsolete">Largeur:</translation>
    </message>
    <message>
        <source>Height :</source>
        <translation type="obsolete">Hauteur:</translation>
    </message>
    <message>
        <source>vdpau</source>
        <translation type="obsolete">vdpau</translation>
    </message>
</context>
<context>
    <name>vdpaudeint</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/vdpauFilters/ADM_vidVdpauFilterDeint.cpp" line="+131"/>
        <source>vdpauDeint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>VDPAU deinterlacer (+resize).</source>
        <translation type="unfinished">VDPAU de-entrelacement (+redimensionnement).</translation>
    </message>
    <message>
        <location line="+185"/>
        <source>Keep Top Field</source>
        <translation type="unfinished">Garder le champs supérieur</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Keep Bottom Field</source>
        <translation type="unfinished">Garder le champs inférieur</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Double framerate</source>
        <translation type="unfinished">Doubler le nombre d&apos;image par seconde</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>_Ivtc:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Resize:</source>
        <translation type="unfinished">_Redimensionner</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Deint Mode:</source>
        <translation type="unfinished">Mode de _deentrelacement:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Width :</source>
        <translation type="unfinished">Largeur:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Height :</source>
        <translation type="unfinished">Hauteur:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>vdpau</source>
        <translation type="unfinished">vdpau</translation>
    </message>
</context>
<context>
    <name>vdpaufilter</name>
    <message>
        <source>Keep Top Field</source>
        <translation type="obsolete">Garder le champs supérieur</translation>
    </message>
    <message>
        <source>Keep Bottom Field</source>
        <translation type="obsolete">Garder le champs inférieur</translation>
    </message>
    <message>
        <source>Double framerate</source>
        <translation type="obsolete">Doubler le nombre d&apos;image par seconde</translation>
    </message>
    <message>
        <source>_Resize:</source>
        <translation type="obsolete">_Redimensionner</translation>
    </message>
    <message>
        <source>_Deint Mode:</source>
        <translation type="obsolete">Mode de _deentrelacement:</translation>
    </message>
    <message>
        <source>Width :</source>
        <translation type="obsolete">Largeur:</translation>
    </message>
    <message>
        <source>Height :</source>
        <translation type="obsolete">Hauteur:</translation>
    </message>
    <message>
        <source>vdpau</source>
        <translation type="obsolete">vdpau</translation>
    </message>
</context>
<context>
    <name>vdpaufilter2</name>
    <message>
        <source>Width :</source>
        <translation type="obsolete">Largeur:</translation>
    </message>
    <message>
        <source>Height :</source>
        <translation type="obsolete">Hauteur:</translation>
    </message>
    <message>
        <source>vdpau</source>
        <translation type="obsolete">vdpau</translation>
    </message>
</context>
<context>
    <name>vdpresize</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/vdpauFilters/ADM_vidVdpauFilter.cpp" line="+69"/>
        <source>vdpau: Resize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>vdpau: Resize image using vdpau.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+133"/>
        <source>Width :</source>
        <translation type="unfinished">Largeur:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Height :</source>
        <translation type="unfinished">Hauteur:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>vdpau</source>
        <translation type="unfinished">vdpau</translation>
    </message>
</context>
<context>
    <name>vflip</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/verticalFlip/verticalFlip.cpp" line="+47"/>
        <source>Vertical Flip</source>
        <translation type="unfinished">Inversion verticale</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertically flip the image.</source>
        <translation type="unfinished">Inverse l&apos;image verticalement.</translation>
    </message>
</context>
<context>
    <name>vidColorYuv</name>
    <message>
        <source>None</source>
        <translation type="obsolete">Aucun</translation>
    </message>
</context>
<context>
    <name>vobSubDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/vobsub.ui" line="+13"/>
        <source>Subtitle Font Size and Position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Delay :</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Select File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Language :</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Idx File :</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>vorbis</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_audioEncoders/vorbis/audioencoder_vorbis.cpp" line="+350"/>
        <source>_Quality:</source>
        <translation type="unfinished">_Qualité:</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>VBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quality based</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>_Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vorbis Configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>vsWindow</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_demuxers/VapourSynth/qt4/vsProxy_qt4.cpp" line="+68"/>
        <source>Open VapourSynth File 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>VS File Files (*.vpy)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>webmmuxer</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerWebm/muxerWebm.cpp" line="+66"/>
        <source>Unsupported Video.
Only VP8/VP9 video and Vorbis/Opus audio supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Unsupported Audio.
Only VP8/VP9 video and Vorbis/Opus audio supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Saving Webm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_muxers/muxerWebm/muxerWebmConfig.cpp" line="+27"/>
        <source>Force display width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Webm Muxer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>workingDialog</name>
    <message>
        <location filename="../ADM_userInterfaces/ADM_dialog/working.ui" line="+13"/>
        <source>Processing</source>
        <translation type="unfinished">En cours</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Elapsed:</source>
        <translation type="unfinished">Temps écoulé:</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Time Remaining:</source>
        <translation type="unfinished">Temps Restant:</translation>
    </message>
    <message>
        <location line="+113"/>
        <source>Cancel</source>
        <translation type="unfinished">Annuler</translation>
    </message>
    <message>
        <location line="-87"/>
        <source>00:00:00</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>x264</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x264/ADM_x264Setup.cpp" line="+182"/>
        <source>Not coded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>this mode has not been implemented
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x264/qt4/Q_x264.cpp" line="+206"/>
        <source>Custom</source>
        <translation type="unfinished">Manuel</translation>
    </message>
    <message>
        <location line="+411"/>
        <source>Target Bitrate:</source>
        <translation type="unfinished">Bitrate cible :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+19"/>
        <source>kbit/s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-15"/>
        <source>Quantiser:</source>
        <translation type="unfinished">Quantisation:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Quality:</source>
        <translation type="unfinished">Qualité:</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Target Video Size:</source>
        <translation type="unfinished">Taille vidéo cible:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Average Bitrate:</source>
        <translation type="unfinished">Bitrate moyen:</translation>
    </message>
    <message>
        <location line="+68"/>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.

Do you wish to continue?</source>
        <translation type="unfinished">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.

Do you wish to continue?</source>
        <translation type="unfinished">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <location line="+41"/>
        <location line="+69"/>
        <location line="+14"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="-83"/>
        <source>Cannot load preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Save Profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>my profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Overwrite</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Replace the following preset ?:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Cannot save preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Cannot delete custom profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Do you really want to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source> profile ?.
If it is a system profile it will be recreated next time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Delete</source>
        <translation type="unfinished">Effacer</translation>
    </message>
</context>
<context>
    <name>x264ConfigDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x264/qt4/x264ConfigDialog.ui" line="+15"/>
        <source>x264 Configuraton</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Configuration:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Save As</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <location line="+3152"/>
        <source>Delete</source>
        <translation type="unfinished">Effacer</translation>
    </message>
    <message>
        <location line="-3110"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Basic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Use advanced configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Profile:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Preset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+51"/>
        <source>Tuning:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>IDC Level:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <location line="+1149"/>
        <source>Auto</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1144"/>
        <location line="+669"/>
        <source>1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-664"/>
        <source>1.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>1.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>1.3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+639"/>
        <source>3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-634"/>
        <source>3.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>3.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>5</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>5.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Fast Decode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Zero Latency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Fast First Pass</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Threads </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Target Video Size:</source>
        <translation type="unfinished">Taille vidéo cible:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Constant Bitrate (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Constant Quantiser (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Constant Rate Factor (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Video Size (Two Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Average Bitrate (Two Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Encoding Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>MB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+60"/>
        <location line="+200"/>
        <source>0 (High Quality)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-177"/>
        <location line="+200"/>
        <location line="+1649"/>
        <source>Quantiser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1826"/>
        <location line="+200"/>
        <source>51 (Low Quality)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-101"/>
        <source>Advanced RC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Advanced Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Maximum Constant Rate Factor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+158"/>
        <source>Macroblock-tree Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Frametype Lookahead:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+2490"/>
        <source>frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-2466"/>
        <source>Sequence Parameter Set Identifer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>7</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>15</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>31</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Motion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Motion Estimation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Motion Estimation Method:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Diamond Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Hexagonal Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Uneven Multi-hexagonal Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Exhaustive Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Hadamard Exhaustive Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>1 (Fast)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Subpixel Refinement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>10 (Best)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+78"/>
        <source>Motion Vector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Maximum Motion Vector Length:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Minimum Buffer Between Threads:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maximum Motion Vector Search Range:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Prediction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+450"/>
        <location line="+36"/>
        <source>Disabled</source>
        <translation type="unfinished">Désactivé</translation>
    </message>
    <message>
        <location line="-481"/>
        <source>Weighted References</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Weighted References + Duplicates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Direct Prediction Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Weighted Prediction for P-frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <location line="+2596"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="-2591"/>
        <source>Spatial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Temporal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Weighted Prediction for B-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Constrained Intra Prediction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Partition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Partition Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>8x8 DCT Spatial Transform</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>8x8, 8x16 and 16x8 P-frame Intra-predicted Blocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>8x8, 8x16 and 16x8 B-frame Intra-predicted Blocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>4x4, 4x8 and 8x4 P-frame Intra-predicted Blocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>8x8 Intra-predicted Blocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>4x4 Intra-predicted Blocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Frame Encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>CABAC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Loop Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <location line="+1328"/>
        <source>Strength:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1289"/>
        <source>Threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Open GOP:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Normal Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Blu-ray Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Interlaced:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Bottom Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Top Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Maximum Reference Frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+39"/>
        <source>B-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>B-frames as References:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Strictly Hierarchical Pyramid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Non-strict (Not Blu-ray Compatible)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Fast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Optimal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+74"/>
        <source>B-frame Bias:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maximum Consecutive B-frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Adaptive B-frame Decision:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>I-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>GOP Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Minimum:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Maximum:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+32"/>
        <source>I-frame Threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Periodic Intra Refresh</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <location line="+6"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Mixed References</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Chroma Motion Estimation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Trellis Quantization:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Final Macroblock Only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Always On</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Fast Skip Detection on P-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>DCT Decimation on P-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Psychovisual Rate Distortion Optimisation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Psychovisual Trellis:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Noise Reduction:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Luma Quantisation Deadzone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Intra Luma Quantisation Deadzone:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Inter Luma Quantisation Deadzone:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+66"/>
        <source>Quantisation Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Flat Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>JVT Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Custom Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+856"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-809"/>
        <source>Quantiser Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Maximum Quantiser:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I and P-frame Quantiser Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+85"/>
        <location line="+179"/>
        <location line="+302"/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-459"/>
        <source>Maximum Quantiser Step:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>P and B-frame Quantiser Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Average Bitrate Tolerance:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Chroma to Luma Quantiser Offset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Minimum Quantiser:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+48"/>
        <source>Quantiser Curve Compression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Quantiser Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Reduce Fluctuation Before Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Reduce Fluctuation After Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Adaptive Quantisation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Variance AQ:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Normal</source>
        <translation type="unfinished">Normale</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Auto Variance AQ</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+91"/>
        <source>Advanced 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Video Buffer Verifier</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Maximum VBV Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Initial VBV Buffer Occupancy:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>kbit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+65"/>
        <source>kbit/s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>VBV Buffer Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Slicing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Slices per Frame:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maximum Size per Slice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>bytes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Maximum Macroblocks per Slice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+58"/>
        <source>Zones</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Add</source>
        <translation type="unfinished">Ajouter</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Advanced 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Multithreading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Disable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Auto-detect</source>
        <translation type="unfinished">Détection auto</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Custom:</source>
        <comment>multithreading</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Enforce Repeatability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Slice-based Threading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Custom Threaded Lookahead Buffer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+59"/>
        <source>Output 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Output</source>
        <translation type="unfinished">Sortie</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Generate Access Unit Delimiters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>BluRay compatibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Generate fake interlaced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Pixel Aspect Ratio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Custom:</source>
        <comment>PAR</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>As Input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+49"/>
        <source>:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Predefined Aspect Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>16:15 (PAL 4:3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>64:45 (PAL 16:9)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>8:9 (NTSC 4:3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>32:27 (NTSC 16:9)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Output 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Video Usability Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>These settings are only suggestions for the playback equipment.  Use at your own risk.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Overscan:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+46"/>
        <location line="+58"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>Undefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-235"/>
        <source>Show</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Crop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Video Format:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Component</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>PAL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>NTSC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>SECAM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>MAC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Colour Primaries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>BT709</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+63"/>
        <source>BT470M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>BT470BG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+136"/>
        <source>SMPTE170M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+78"/>
        <location line="+58"/>
        <source>SMPTE240M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <source>Film</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Transfer Characteristics:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Linear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>LOG100</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>LOG316</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>SMPTEL170M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Colour Matrix:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>FCC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>GBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>YCgCo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>HRD Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>VBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>CBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Chroma Sample Location:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Full Range Samples</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>x264CustomMatrixDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x264/qt4/x264CustomMatrixDialog.ui" line="+16"/>
        <source>x264 Custom Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Intra 4x4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <location line="+446"/>
        <location line="+446"/>
        <location line="+703"/>
        <source>Luma:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1385"/>
        <location line="+446"/>
        <source>Chroma U/V:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-238"/>
        <source>Inter 4x4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+446"/>
        <source>Intra 8x8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+703"/>
        <source>Inter 8x8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+727"/>
        <source>Load File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Cancel</source>
        <translation type="unfinished">Annuler</translation>
    </message>
</context>
<context>
    <name>x264Dialog</name>
    <message>
        <source>Target Bitrate:</source>
        <translation type="obsolete">Bitrate cible :</translation>
    </message>
    <message>
        <source>Quantiser:</source>
        <translation type="obsolete">Quantisation:</translation>
    </message>
    <message>
        <source>Quality:</source>
        <translation type="obsolete">Qualité:</translation>
    </message>
    <message>
        <source>Target Video Size:</source>
        <translation type="obsolete">Taille vidéo cible:</translation>
    </message>
    <message>
        <source>Average Bitrate:</source>
        <translation type="obsolete">Bitrate moyen:</translation>
    </message>
    <message>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.

Do you wish to continue?</source>
        <translation type="obsolete">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.

Do you wish to continue?</source>
        <translation type="obsolete">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
</context>
<context>
    <name>x265</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x265/ADM_x265Setup.cpp" line="+158"/>
        <source>Not coded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>this mode has not been implemented
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x265/qt4/Q_x265.cpp" line="+212"/>
        <source>Custom</source>
        <translation type="unfinished">Manuel</translation>
    </message>
    <message>
        <location line="+357"/>
        <source>Target Bitrate:</source>
        <translation type="unfinished">Bitrate cible :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+20"/>
        <source>kbit/s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-15"/>
        <source>Quantiser:</source>
        <translation type="unfinished">Quantisation:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Quality:</source>
        <translation type="unfinished">Qualité:</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Target Video Size:</source>
        <translation type="unfinished">Taille vidéo cible:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Average Bitrate:</source>
        <translation type="unfinished">Bitrate moyen:</translation>
    </message>
    <message>
        <location line="+63"/>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.

Do you wish to continue?</source>
        <translation type="unfinished">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.

Do you wish to continue?</source>
        <translation type="unfinished">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <location line="+41"/>
        <location line="+69"/>
        <location line="+14"/>
        <source>Error</source>
        <translation type="unfinished">Erreur</translation>
    </message>
    <message>
        <location line="-83"/>
        <source>Cannot load preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Save Profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Overwrite</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Replace the following preset ?:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Cannot save preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Cannot delete custom profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Do you really want to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source> profile ?.
If it is a system profile it will be recreated next time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete preset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Delete</source>
        <translation type="unfinished">Effacer</translation>
    </message>
</context>
<context>
    <name>x265ConfigDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x265/qt4/x265ConfigDialog.ui" line="+15"/>
        <source>x265 Configuraton</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Configuration:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Save As</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <location line="+2731"/>
        <source>Delete</source>
        <translation type="unfinished">Effacer</translation>
    </message>
    <message>
        <location line="-2689"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Basic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Use advanced configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Profile:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Preset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+51"/>
        <source>Tuning:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>IDC Level:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Auto</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+668"/>
        <source>1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-663"/>
        <source>1.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>1.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>1.3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>2.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+638"/>
        <source>3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-633"/>
        <source>3.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>3.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>4.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>5</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>5.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Pool Threads </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Frame Threads </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Target Video Size:</source>
        <translation type="unfinished">Taille vidéo cible:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Constant Bitrate (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Constant Quantiser (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Constant Rate Factor (Single Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Video Size (Two Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Average Bitrate (Two Pass)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Encoding Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>MB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+213"/>
        <source>0 (High Quality)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+213"/>
        <location line="+1267"/>
        <source>Quantiser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1457"/>
        <location line="+213"/>
        <source>51 (Low Quality)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-143"/>
        <source>Use Strict Constant Bitrate Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Advanced RC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Advanced Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Maximum Constant Rate Factor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+158"/>
        <source>Coding Unit-tree Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Frametype Lookahead:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+2070"/>
        <source>frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-2046"/>
        <source>Sequence Parameter Set Identifer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>7</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>15</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>31</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Motion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Motion Estimation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Motion Estimation Method:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Diamond Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Hexagonal Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Uneven Multi-hexagonal Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Star Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Exhaustive Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>1 (Fast)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Subpixel Refinement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>7 (Best)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+78"/>
        <source>Motion Vector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Maximum Motion Vector Search Range:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Prediction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Weighted Prediction for B-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Constrained Intra Prediction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Weighted Prediction for P-frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+217"/>
        <location line="+36"/>
        <source>Disabled</source>
        <translation type="unfinished">Désactivé</translation>
    </message>
    <message>
        <location line="-248"/>
        <source>Weighted References</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Weighted References + Duplicates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Frame Encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Loop Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Open GOP:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Normal Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Blu-ray Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Interlaced:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Top Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Bottom Field First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Maximum Reference Frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+39"/>
        <source>B-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>B-frames as References:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Strictly Hierarchical Pyramid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Non-strict (Not Blu-ray Compatible)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Fast</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Optimal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+74"/>
        <source>B-frame Bias:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maximum Consecutive B-frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Adaptive B-frame Decision:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>I-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>GOP Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Minimum:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Maximum:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+32"/>
        <source>I-frame Threshold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+53"/>
        <location line="+6"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Trellis Quantization:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Final Macroblock Only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Always On</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Fast Skip Detection on P-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>DCT Decimation on P-frames</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Psychovisual Rate Distortion Optimisation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Noise Reduction:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Noise Reduction Intra:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Noise Reduction Inter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Quantisation Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Flat Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>JVT Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Custom Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+818"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-771"/>
        <source>Quantiser Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Chroma to Luma Quantiser Offset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+34"/>
        <source>I and P-frame Quantiser Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+58"/>
        <location line="+168"/>
        <location line="+302"/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-448"/>
        <source>Maximum Quantiser Step:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Cb</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Cr</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>P and B-frame Quantiser Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Average Bitrate Tolerance:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+48"/>
        <source>Quantiser Curve Compression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Quantiser Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Reduce Fluctuation Before Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Reduce Fluctuation After Curve Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Adaptive Quantisation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Variance AQ:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Normal</source>
        <translation type="unfinished">Normale</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Auto Variance AQ</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Strength:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+65"/>
        <source>Advanced 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Video Buffer Verifier</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Maximum VBV Bitrate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Initial VBV Buffer Occupancy:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>kbit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+65"/>
        <source>kbit/s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>VBV Buffer Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Slicing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Slices per Frame:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maximum Size per Slice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>bytes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Maximum Macroblocks per Slice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+58"/>
        <source>Zones</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Add</source>
        <translation type="unfinished">Ajouter</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Advanced 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Multithreading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Disable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Auto-detect</source>
        <translation type="unfinished">Détection auto</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Custom:</source>
        <comment>multithreading</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Enforce Repeatability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Slice-based Threading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Custom Threaded Lookahead Buffer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+59"/>
        <source>Output 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Output</source>
        <translation type="unfinished">Sortie</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Generate Access Unit Delimiters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Pixel Aspect Ratio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Custom:</source>
        <comment>PAR</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>As Input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+49"/>
        <source>:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Predefined Aspect Ratio:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>16:15 (PAL 4:3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>64:45 (PAL 16:9)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>8:9 (NTSC 4:3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>32:27 (NTSC 16:9)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Output 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Video Usability Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>These settings are only suggestions for the playback equipment.  Use at your own risk.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Overscan:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+46"/>
        <location line="+58"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>Undefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-235"/>
        <source>Show</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Crop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Video Format:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Component</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>PAL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>NTSC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>SECAM</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>MAC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Colour Primaries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>BT709</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+63"/>
        <source>BT470M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+63"/>
        <location line="+73"/>
        <source>BT470BG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+136"/>
        <source>SMPTE170M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <location line="+78"/>
        <location line="+58"/>
        <source>SMPTE240M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-131"/>
        <source>Film</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Transfer Characteristics:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Linear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>LOG100</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>LOG316</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>SMPTEL170M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Colour Matrix:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>FCC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>GBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>YCgCo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>HRD Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>VBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>CBR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Chroma Sample Location:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Full Range Samples</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>x265CustomMatrixDialog</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/x265/qt4/x265CustomMatrixDialog.ui" line="+14"/>
        <source>x265 Custom Matrix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Intra 4x4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+28"/>
        <location line="+446"/>
        <location line="+446"/>
        <location line="+703"/>
        <source>Luma:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1385"/>
        <location line="+446"/>
        <source>Chroma U/V:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-238"/>
        <source>Inter 4x4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+446"/>
        <source>Intra 8x8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+703"/>
        <source>Inter 8x8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+727"/>
        <source>Load File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Cancel</source>
        <translation type="unfinished">Annuler</translation>
    </message>
</context>
<context>
    <name>x265Dialog</name>
    <message>
        <source>Target Bitrate:</source>
        <translation type="obsolete">Bitrate cible :</translation>
    </message>
    <message>
        <source>Quantiser:</source>
        <translation type="obsolete">Quantisation:</translation>
    </message>
    <message>
        <source>Quality:</source>
        <translation type="obsolete">Qualité:</translation>
    </message>
    <message>
        <source>Target Video Size:</source>
        <translation type="obsolete">Taille vidéo cible:</translation>
    </message>
    <message>
        <source>Average Bitrate:</source>
        <translation type="obsolete">Bitrate moyen:</translation>
    </message>
    <message>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.

Do you wish to continue?</source>
        <translation type="obsolete">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
    <message>
        <source>Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.

Do you wish to continue?</source>
        <translation type="obsolete">L&apos; optimisation Macroblock-Tree  nécessite  Variance Adaptive Quantisation. Variance Adaptive Quantisation va être automatiquement activée.

Voulez vous procéder ?</translation>
    </message>
</context>
<context>
    <name>xvid4</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoEncoder/xvid4/xvid4Dialog.cpp" line="+37"/>
        <location line="+24"/>
        <source>None</source>
        <translation type="unfinished">Aucun</translation>
    </message>
    <message>
        <location line="-23"/>
        <source>Low</source>
        <translation type="unfinished">BAsse</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Medium</source>
        <translation type="unfinished">Moyenne</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Full</source>
        <translation type="unfinished">Complète</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Custom</source>
        <translation type="unfinished">Manuel</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>One thread</source>
        <translation type="unfinished">Un thread</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Two threads)</source>
        <translation type="unfinished">Deux threads</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Three threads</source>
        <translation type="unfinished">Trois threads</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>MotionEstimation</source>
        <translation type="unfinished">Estimation de mouvement</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Aspect Ratio:</source>
        <translation type="unfinished">Rapport h/l:</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Aspect Ratio</source>
        <translation type="unfinished">Rapport hauteur/largeur</translation>
    </message>
    <message>
        <location line="-101"/>
        <source>H.263</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>MPEG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+17"/>
        <source>DCT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Qpel16</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Qpel8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Square</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Auto (#cpu)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>1:1 (PC)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>4:3 (PAL))</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>4:3 (NTSC))</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>16:9 (PAL))</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>16:9 (NTSC))</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Threading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Mi_n. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Ma_x. quantizer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>_Trellis quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Number of B frames:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Quantization type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>_Macroblock decision:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Profile:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>_Gop Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Use XVID fcc (else DIVX)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Advanced Simple Profile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Motion Estimation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Quantization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Threads</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Rate Control</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Xvid4 MPEG-4 ASP configuration</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>yadif</name>
    <message>
        <location filename="../../../avidemux_plugins/ADM_videoFilters6/yadif/ADM_vidYadif.cpp" line="+77"/>
        <source>Yadif</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Yadif, port of avisynth version (c) Fizick.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Temporal &amp; spatial check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bob, temporal &amp; spatial check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Skip spatial temporal check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bob, skip spatial temporal check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Bottom field first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Top field first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>_Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>_Order:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>yadif</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
