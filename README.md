# RC-Car
A client server rc car built using zmq and protobuf meant for a raspberry pi connected to motors

## GitHub CI + Docker
This repository now builds `car_sim`, `controller_sim`, and `visualizer_sim` on push and produces multi-architecture Docker images for `linux/amd64` and `linux/arm64`.

### Pull and run
Images are published to GitHub Container Registry under `ghcr.io/<owner>/rc-car/<service>`.

Example commands:

```sh
docker pull ghcr.io/<owner>/rc-car/car_sim:latest
docker run --rm ghcr.io/<owner>/rc-car/car_sim:latest
```

Replace `<owner>` with your GitHub user or org.

### Local build and run
Build one image locally with:

```sh
docker buildx build --platform linux/amd64,linux/arm64 --build-arg TARGET_EXECUTABLE=car_sim -t car_sim:local .
```

Run it with:

```sh
docker run --rm car_sim:local
```
