#
echo "Dont forget devpod provider set-options --provider docker --option INACTIVITY_TIMEOUT=90m"
devpod up . --ide none --id adm-mxe64  --devcontainer-path .devcontainer_mxe64/devcontainer.json
