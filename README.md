# Height23D

## Note
This project has not been finished and there are still major flaws need to be address. 

## Introduction
Height23D is a project aimed to convert height map into polygon base on the height contour line. The idea is to create models with minimum vertices/edges while maintaining the feature of origianl height map even under many universal smoothing algorithm. 

> A metaphor about the ultimate goal of the project.
![ConvertIdea](https://github.com/biohazardlby/Height23D/blob/master/ReadMeImage/ConverIdea.png)

## Pipeline
The pipeline is designed as follow:
![Pipeline Image](https://github.com/biohazardlby/Height23D/blob/master/ReadMeImage/pipeline.png)
The process marked in blue is already introduced to the proejct, while the other is to be finished.

## Library used
* OpenCV

## Demonstration Image:
Original Heightmap:

![Original Heightmap](https://github.com/biohazardlby/Height23D/blob/master/Height2Terrain/hm2.png)

Contour Generated:

![Contour Generated](https://github.com/biohazardlby/Height23D/blob/master/ReadMeImage/Process1.png)

Connect Contour:

![Contour Generated](https://github.com/biohazardlby/Height23D/blob/master/ReadMeImage/Process2.png)

## Command
* Mouse Click on Original Heightmap: Select Grayscale
* S key:  Simplify contour
* Spacebar: Save simplified contour
* Enter:  Connect saved contours

## Known Issue:
Without proper uniform generating pattern for the contour to follow, connecting the simplified contours can barely produce satifying result.

## To Work in future:
Recreate the simplification pipeline so each contour follow the same pattern.

## Video Demo
https://www.youtube.com/watch?v=Sm4LW_NcCNY
