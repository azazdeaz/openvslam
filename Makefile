

install-dependencies:
	sh ./scripts/install_dependencies.sh

run-cmake:
	mkdir -p build && \
	cd build && \
	cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_UNIT_TESTS=OFF \
	-DBUILD_EXAMPLES=OFF \
	-DBUILD_WITH_MARCH_NATIVE=ON \
	-DBOW_FRAMEWORK=FBoW \
	-DUSE_SOCKET_PUBLISHER=OFF \
	-DCMAKE_INSTALL_PREFIX=/home/azazdeaz/repos/good-bug/openvslam-wrap/openvslam/dependencies \
	..

build-api:
	cd build && make -j3

run-api:
	cd build && make -j3 && ./run_api -c ../../config/cfg.yaml -v ../../config/orb_vocab.fbow -m ../../config/dataset/aist_living_lab_1/video.mp4

run-api-video:
	cd build && make -j3 && ./run_api -c ../../config/dataset/aist_living_lab_1/config.yaml -v ../../config/orb_vocab.fbow -m ../../config/dataset/aist_living_lab_1/video.mp4

bot:
	cd build && make -j3 && ./run_csi_image_slam -v ../../config/orb_vocab.fbow -c ../../config/leopard640x480.yaml  --mask ../../config/mask1.png # --map-db-out map1b.msg # --map-db-in map1.msg  --map-db-out map2.msg
