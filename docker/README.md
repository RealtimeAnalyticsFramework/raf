
How to start a container by startIdgsContainer?
startIdgsContainer need 3 arguments: image id, conainer name, ip/mask
- images id: the image id must be selected from the runtime images.
- container name: must followed by a number, the number identify the order to start the container.
- ip/mask: such as 10.1.42.11/16

here is an example to start a raf container.
  ./startIdgsContainer raf:v3 idgs1 10.1.42.11/16
