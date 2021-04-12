dev:
	cd build && make -j8 && ./run_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheyehd.config.yaml -i ../../dataset/fish/fish4

test1:
	cd build && make -j8 && ./run_video_slam -v orb_vocab.dbow2 -m ./aist_living_lab_1/video.mp4 -c ./aist_living_lab_1/config.yaml -p lab.map.msg

bot:
	cd build && make -j8 && ./run_zmq_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheyehd.config.yaml  -p map.msg
