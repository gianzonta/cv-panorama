mkdir .\comp

for %%F in (*.JPG) do (

    ffmpeg -i %%F  -vf scale=-1:400 comp/%%F.png
)