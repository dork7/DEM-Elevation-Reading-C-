#include <iostream>
#include <vector>
#include "gdal/gdal_priv.h"
#include "gdal/cpl_conv.h"

int main() {
    // Initialize GDAL
    GDALAllRegister();
    
    // Open the file
    GDALDataset  *dataset = (GDALDataset *) GDALOpen("E075_N35.tif", GA_ReadOnly);
    if( dataset == NULL ) {
      std::cout << "Failed to open" << std::endl;
      exit(1);
    }
    
    // Get image metadata
    unsigned width = dataset->GetRasterXSize();
    unsigned height = dataset->GetRasterYSize();
    
    std::cout << "Image is " << width << " x " << height << " px" << std::endl;
    
    double originX, originY, pixelSizeX, pixelSizeY;
    
    double geoTransform[6];
    if (dataset->GetGeoTransform(geoTransform) == CE_None ) {
        originX = geoTransform[0];
        originY = geoTransform[3];
        pixelSizeX = geoTransform[1];
        pixelSizeY = geoTransform[5];
    } else {
      std::cout << "Failed read geotransform" << std::endl;
      exit(1);
    }
    
    std::cout << "Origin: " << originX << ", " << originY << " degrees" << std::endl;
    std::cout << "Pixel size: " << pixelSizeX << ", " << pixelSizeY << " degrees" << std::endl; 
    
    // pixelSizeY is negative because the origin of the image is the north-east corner and positive
    // Y pixel coordinates go towards the south
    
    // Get the raster band (DEM has one raster band representing elevation)
    GDALRasterBand  *elevationBand = dataset->GetRasterBand(1);
    
    // Create an array of width*height 32-bit floats (~400MB memory)
    std::vector<float> data(width * height, 0.0f);
   
    // Read the entire file into the array (you can change the options to read only a portion
    // of the file, or even scale it down if you want)
     
    std::cout << "Loading array..." << std::flush;
    elevationBand->RasterIO(GF_Read, 0, 0, width, height, &data[0], width, height, GDT_Float32, 0, 0);
    std::cout << "done" << std::endl;
    
    // Close the file
    GDALClose(dataset); 
    
    // Be careful with axis order: coordinates are traditionally written in lat, lon order, but
    // those are Y, X, respectively.    
    double pointLat = 35.5624835;
    double pointLon = 75.4695603;
    
    double pixelX = (pointLon - originX) / pixelSizeX;
    double pixelY = (pointLat - originY) / pixelSizeY;
    
    std::cout << pointLat << "," << pointLon << " maps to pixel " << pixelX << "," << pixelY << std::endl;
    
    if (pixelX < 0 || pixelX > width || pixelY < 0 || pixelY > height) {
      std::cout << "Point out of bounds!" << std::endl;
      exit(1);
    }
    
    double elev = data[int(pixelX) + int(pixelY) * width];
    
    std::cout << "Elevation there is " << elev << "m" << std::endl;
}