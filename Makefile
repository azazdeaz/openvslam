run-cmake:
	cd build && \
	cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=/usr/local \
	-DBUILD_UNIT_TESTS=OFF \
	-DBUILD_EXAMPLES=ON \
	-DBOW_FRAMEWORK=FBoW \
	-DUSE_SOCKET_PUBLISHER=ON \
	-DCMAKE_INSTALL_PREFIX=/home/azazdeaz/repos/good-bug/openvslam-wrap/openvslam/dependencies \
	..

bot:
	cd build && make -j3 && ./run_csi_image_slam -v ../../config/orb_vocab.fbow -c ../../config/picam640x480wide.yaml  --mask ../../config/mask2.png # --map-db-out map1b.msg # --map-db-in map1.msg  --map-db-out map2.msg
