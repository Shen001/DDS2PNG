# DDS2PNG
convert osg textures .dds file to png 将DDS转换为PNG

### 说明
1.在将osg模型转换为obj模型时，发现模型绕着X轴顺时针旋转了90度，所有代码不仅仅转换了图片格式，还将模型进行了逆时针旋转90度；
2.使用了微软的开源c++库 DirectXTex进行图片的转换，但是转换的图片被进行了镜像处理，所以我也不知道为啥，只好使用Opencv图片库将图片镜像了回来
3.生成的obj的纹理文件.mtl是文本格式，里面的图片格式需要c++读取之后修改

### 处理文件
*输入：*.osg .dds
*输出：*.obj .png

### 使用的库
1. DirectXTex.exe(这个exe是我使用c++2017编译生成的)
2. opencv
3. openscenegraph
