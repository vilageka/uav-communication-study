#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/ns-3-dev" >&2
    exit 1
fi

NS3_DIR="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [ ! -x "${NS3_DIR}/ns3" ]; then
    echo "Error: ${NS3_DIR} does not look like an ns-3 checkout." >&2
    echo "Expected executable file: ${NS3_DIR}/ns3" >&2
    exit 1
fi

mkdir -p "${NS3_DIR}/scratch"
mkdir -p "${NS3_DIR}/scripts"

cp "${REPO_DIR}/scratch/uav-test.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-wifi-baseline.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-wifi-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-mesh-olsr-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-lte-infrastructure-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-urban-wifi-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-urban-mesh-olsr-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scratch/uav-urban-lte-infrastructure-aoi.cc" "${NS3_DIR}/scratch/"
cp "${REPO_DIR}/scripts/uav-run-experiments.py" "${NS3_DIR}/scripts/"
cp "${REPO_DIR}/scripts/uav-analyze-results.py" "${NS3_DIR}/scripts/"
cp "${REPO_DIR}/scripts/uav-build-report.py" "${NS3_DIR}/scripts/"
chmod +x "${NS3_DIR}/scripts/uav-run-experiments.py"
chmod +x "${NS3_DIR}/scripts/uav-analyze-results.py"
chmod +x "${NS3_DIR}/scripts/uav-build-report.py"

echo "Installed UAV scratch programs to ${NS3_DIR}/scratch"
echo "Installed UAV experiment runner to ${NS3_DIR}/scripts"
echo
echo "Run them from the ns-3 directory, for example:"
echo "  cd ${NS3_DIR}"
echo "  ./ns3 run uav-test"
echo "  ./ns3 run \"uav-wifi-baseline --numUavs=20 --spacing=100\""
echo "  ./ns3 run \"uav-wifi-aoi --numUavs=20 --spacing=100 --aoiSampleInterval=0.1\""
echo "  ./ns3 run \"uav-mesh-olsr-aoi --numUavs=20 --spacing=100 --appStart=5\""
echo "  ./ns3 run \"uav-lte-infrastructure-aoi --numUavs=20 --spacing=100 --appStart=1\""
echo "  ./ns3 run \"uav-urban-wifi-aoi --numUavs=20 --spacing=100\""
echo "  ./ns3 run \"uav-urban-mesh-olsr-aoi --numUavs=20 --spacing=100 --appStart=5\""
echo "  ./ns3 run \"uav-urban-lte-infrastructure-aoi --numUavs=20 --spacing=100 --appStart=1\""
echo "  scripts/uav-run-experiments.py --profile standard"
echo "  scripts/uav-analyze-results.py results/uav-urban-all-v08"
