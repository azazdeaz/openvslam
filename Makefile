dev:
	cd build && make -j8 && ./run_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheyehd.config.yaml -i ../../dataset/fish/fish4

test1:
	cd build && make -j8 && ./run_video_slam -v orb_vocab.dbow2 -m ./aist_living_lab_1/video.mp4 -c ./aist_living_lab_1/config.yaml -p lab.map.msg

bot_hd:
	cd build && make -j8 && ./run_zmq_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheyehd.config.yaml  #-p map.msg

bot:
	cd build && make -j8 && ./run_zmq_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheye.640x480.aspersp.config.yaml --mask ../../dataset/mask2.png  --map-db-out map1b.msg # --map-db-in map1.msg  --map-db-out map2.msg

bot_picam:
	cd build && make -j8 && ./run_zmq_image_slam -v orb_vocab.dbow2 -c ../../dataset/picam640x480wide.yaml --mask ../../dataset/mask3.png   --map-db-out map1.msg --map-db-in map1.msg # --map-db-out map2.msg
	

gazebo:
	cd build && make -j8 && ./run_zmq_image_slam -v orb_vocab.dbow2 -c ../../dataset/gazebohd.config.yaml  #-p map.msg

bot_off:
	cd build && make -j8 && ./run_image_slam -v orb_vocab.dbow2 -c ../../dataset/fisheye.640x480.aspersp.config.yaml -i ../../dataset/boxsh	oe_01
