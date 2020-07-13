// PCL lib Functions for processing point clouds

#include "processPointClouds.h"
#include <unordered_set>
// #include "kdtree3d.h"
// #include "quiz/ransac/ransac2d.cpp"

//constructor:
template <typename PointT>
ProcessPointClouds<PointT>::ProcessPointClouds() {}

//de-constructor:
template <typename PointT>
ProcessPointClouds<PointT>::~ProcessPointClouds() {}

template <typename PointT>
void ProcessPointClouds<PointT>::numPoints(typename pcl::PointCloud<PointT>::Ptr cloud)
{
  std::cout << cloud->points.size() << std::endl;
}

template <typename PointT>
typename pcl::PointCloud<PointT>::Ptr ProcessPointClouds<PointT>::FilterCloud(typename pcl::PointCloud<PointT>::Ptr cloud, float filterRes, Eigen::Vector4f minPoint, Eigen::Vector4f maxPoint)
{

  // Time segmentation process
  auto startTime = std::chrono::steady_clock::now();

  typename pcl::PointCloud<PointT>::Ptr filterCloud(new pcl::PointCloud<PointT>);

  // typename pcl::PointCloud<PointT>::Ptr filterCloud;
  // TODO:: Fill in the function to do voxel grid point reduction and region based filtering
  // Create the filtering object
  pcl::VoxelGrid<PointT> sor;
  sor.setInputCloud(cloud);
  sor.setLeafSize(filterRes, filterRes, filterRes);
  sor.filter(*filterCloud);
  // sor.filter (*cloud_filtered);

  // typename pcl::PointCloud<PointT>::Ptr regionCloud(new pcl::PointCloud<PointT>);
  pcl::CropBox<PointT> cropBox(true);
  // pcl::CropBox<PointT>* cropBox = new pcl::CropBox<PointT> (false);

  cropBox.setInputCloud(filterCloud);
  cropBox.setMax(maxPoint);
  cropBox.setMin(minPoint);
  cropBox.filter(*filterCloud);

  // crop the roof out, from the lesson solution
  std::vector<int> indices;
  pcl::CropBox<PointT> cropRoof(true);
  // pcl::CropBox<PointT>* cropBox = new pcl::CropBox<PointT> (false);

  cropBox.setInputCloud(filterCloud);
  cropBox.setMin(Eigen::Vector4f(-1.5, -1.7, -1, 1));
  cropBox.setMax(Eigen::Vector4f(2.6, 1.7, -0.4, 1));
  cropBox.filter(indices);

  pcl::PointIndices::Ptr inliers{new pcl::PointIndices};

  //iterate through the indices retruned from the roof crop, adding to inliers
  for (int point : indices)
    inliers->indices.push_back(point);

  // TODO: add the roof filer from the solution video
  pcl::ExtractIndices<PointT> extract;
  extract.setInputCloud(filterCloud);
  extract.setIndices(inliers);
  extract.setNegative(true);
  extract.filter(*filterCloud);

  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
  std::cout << "filtering took " << elapsedTime.count() << " milliseconds" << std::endl;

  return filterCloud;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> ProcessPointClouds<PointT>::SeparateClouds(pcl::PointIndices::Ptr inliers, typename pcl::PointCloud<PointT>::Ptr cloud)
{
  // TODO: Create two new point clouds, one cloud with obstacles and other with segmented plane

  typename pcl::PointCloud<PointT>::Ptr obstacleCloud(new pcl::PointCloud<PointT>());
  typename pcl::PointCloud<PointT>::Ptr planeCloud(new pcl::PointCloud<PointT>());

  for (int index : inliers->indices)
  {
    planeCloud->points.push_back(cloud->points[index]);
  }

  pcl::ExtractIndices<PointT> extract;
  extract.setInputCloud(cloud);
  extract.setIndices(inliers);
  extract.setNegative(true);
  extract.filter(*obstacleCloud);

  std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> segResult(obstacleCloud, planeCloud);
  // std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> segResult(planeCloud, cloud);
  return segResult;
}

/// arguments
/// pointer to cloud
/// maxIterations
/// distanceTol increase value to allow wider spaced points to be clustered
template <typename PointT>

// std::unordered_set<int> Ransac1(typename pcl::PointCloud<PointT>::Ptr cloud, int maxIterations, float distanceTol);
std::unordered_set<int> ProcessPointClouds<PointT>::Ransac1(typename pcl::PointCloud<PointT>::Ptr cloud, int maxIterations, float distanceTol)
{
  std::unordered_set<int> inliersResult;
  srand(time(NULL));

  // TODO: Fill in this function

  // number of samples is 2 for the line
  // int goal_samples = 2;

  // Line Formula
  // Ax + By + C = 0
  // (y1-y2)x + (x2-x1)y + (x1*y2 - x2*y1) = 0
  // A = y1-y2
  // B = x2-x1
  // C = (x1*y2 - x2*y1)
  // Point(x,y)
  // Distance d = abs(Ax + By + C) / sqrt((pow(A,2) + (pow(B,2) ))

  // calculate the line coefficients
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered;

  //----------------------------------------------------------------
  // Equation of a Plane through Three Points
  // Ax+By+Cz+D=0
  // point1=(x1,y1,z1)
  // point2=(x2,y2,z2)
  // point3=(x3,y3,z3)

  // Use point1 as a reference and define two vectors on the plane v1 and v2 as follows:

  //     Vector v1 travels from point1 to point2.
  //     Vector v2 travels from point1 to point3

  // model plane using 2 vectors, from point 1 to 2, and point 1 to 3
  // v1=<x2−x1,y2−y1,z2−z1>
  // v2=<x3−x1,y3−y1,z3−z1>

  // d=∣A∗x+B∗y+C∗z+D∣/sqrt(pow(A,2)+pow(B,2) +pow(C,2)).

  // generate 3 random numbers to pick points from the cloud.
  //-----------------------------------------------------------------

  int pointCloudSize = cloud->points.size();

  std::cout << "pointCloud size:\t" << pointCloudSize << std::endl;

  // For max iterations
  for (int index = 0; index < maxIterations; index++)
  {
    std::vector<int> samples;
    std::unordered_set<int> inliers;
    // std::unordered_set<int> samples2;

    // generate 2 random numbers
    int sampleIndex1 = rand() % pointCloudSize;
    int sampleIndex2 = rand() % pointCloudSize;

    // check that the generate random number is unique, and if not then create a new one until it is
    while (sampleIndex1 == sampleIndex2)
      sampleIndex2 = rand() % pointCloudSize;

    // generate a 3rd point for the plane representation
    int sampleIndex3 = rand() % pointCloudSize;

    // check that the generate random number is unique, and if not then create a new one until it is
    while ((sampleIndex1 == sampleIndex3) || (sampleIndex2 == sampleIndex3))
      sampleIndex3 = rand() % pointCloudSize;

    samples.insert(samples.begin(), sampleIndex1);
    // samples2.insert(sampleIndex1);

    std::cout << "samples:\t" << sampleIndex1 << ", " << sampleIndex2 << std::endl;

    // probably better way to do this
    // define variables for x, y, z for each point and populate
    float x1 = cloud->points[sampleIndex1].x;
    float y1 = cloud->points[sampleIndex1].y;
    float z1 = cloud->points[sampleIndex1].z;

    float x2 = cloud->points[sampleIndex2].x;
    float y2 = cloud->points[sampleIndex2].y;
    float z2 = cloud->points[sampleIndex2].z;

    float x3 = cloud->points[sampleIndex3].x;
    float y3 = cloud->points[sampleIndex3].y;
    float z3 = cloud->points[sampleIndex3].z;

    float A = y1 - y2;
    float B = x2 - x1;
    float C = (x1 * y2 - x2 * y1);

    A = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
    B = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
    C = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
    float D = -(A * x1 + B * y1 + C * z1);

    // TODO: would be better to define v1 and v2 using points. ie  v1 = p2 - p1; v2 = p3-p1
    // the resulitng vector form the couple of steps would be v1×v2=<i,j,k>. can then access th eappropriate element
    // the above is the long winded way

    // this dont work but left for ref. THESE DO WORK NOW. ISSUE WITH WRONG MINUS SIGN!!!!
    // std::vector<float> v1{1,2,3};
    // std::vector<float> v1{(x2 - x1),(y2 - y1),(z2 - z1)};
    // std::vector<float> v1{x2 - x1,y2 - y1, z2 - z1};

    std::cout << "A: " << A << " ,B: " << B << ", C: " << C << std::endl;

    for (int j = 0; j < pointCloudSize; j++)
    {

      // check to see if the index was one of the ones selected randomly if not, then check the distance to the line / plane
      if ((j != sampleIndex1) && (j != sampleIndex2))
      {

        float x = cloud->points[j].x;
        float y = cloud->points[j].y;
        float z = cloud->points[j].z;

        // first line is for a line.... second is a plane, commented out line implementation
        // float d = abs(A*x + B*y + C) / sqrt((pow(A,2) + (pow(B,2) )));
        float d = abs(A * x + B * y + C * z + D) / sqrt(pow(A, 2) + pow(B, 2) + pow(C, 2));

        // cout << "index:\t"<< j <<"\t, distance:\t" << d << endl;
        // check if the distance is smaller than the passed tolerance
        if (d < distanceTol)
        {
          inliers.insert(j);
        }
      }
      else
      {
        inliers.insert(j);
      }
    }
    std::cout << "----------------------------" << std::endl;
    std::cout << "Inliers: " << inliersResult.size() << std::endl;

    if (inliers.size() > inliersResult.size())
    {
      inliersResult = inliers;
    }
  }

  // Randomly sample subset and fit line

  // Measure distance between every point and fitted line
  // If distance is smaller than threshold count it as inlier

  // Return indicies of inliers from fitted line with most inliers

  return inliersResult;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> ProcessPointClouds<PointT>::SegmentPlane(typename pcl::PointCloud<PointT>::Ptr cloud, int maxIterations, float distanceThreshold)
{
  // Time segmentation process
  auto startTime = std::chrono::steady_clock::now();

  // pcl::PointCloud<pcl::PointXYZ>::Ptr cloud1;

  // std::unordered_set<int> inliers = Ransac1( cloud,  maxIterations, distanceThreshold);
  std::unordered_set<int> inliers = Ransac1(cloud, 50, 0.15);

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloudInliers(new pcl::PointCloud<pcl::PointXYZ>());
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloudOutliers(new pcl::PointCloud<pcl::PointXYZ>());

  // from the inliers as an ordered set, generate a Pointcloud for the in liers and out liers
  for (int index = 0; index < cloud->points.size(); index++)
  {
    pcl::PointXYZ point = cloud->points[index];
    if (inliers.count(index))
      cloudInliers->points.push_back(point);
    else
      cloudOutliers->points.push_back(point);
  }
  std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> segResult1;

  segResult1.first = cloudInliers;
  segResult1.second = cloudOutliers;

  std::cout << "inliers: " << cloudInliers->size() << ", outliers: " << cloudInliers->size() << std::endl;
  // return cloudInliers, cloudOutliers;

  return segResult1;

  // // original method follows....
  // pcl::PointIndices::Ptr inliersPointIndices {new pcl::PointIndices};
  // TODO:: Fill in this function to find inliers for the cloud.
  // pcl::SACSegmentation<PointT> seg;

  // pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());

  // seg.setOptimizeCoefficients (true);
  // // Mandatory
  // seg.setModelType (pcl::SACMODEL_PLANE);
  // seg.setMethodType (pcl::SAC_RANSAC);
  // seg.setMaxIterations (maxIterations);
  // seg.setDistanceThreshold (distanceThreshold);

  // // Segment the largest planar component from the remaining cloud
  // seg.setInputCloud (cloud);
  // seg.segment (*inliersPointIndices, *coefficients);

  // if (inliersPointIndices->indices.size () == 0)
  // {
  //   std::cerr << "Could not estimate a planar model for the given dataset." << std::endl;
  // }

  // auto endTime = std::chrono::steady_clock::now();
  // auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
  // std::cout << "plane segmentation took " << elapsedTime.count() << " milliseconds" << std::endl;

  // execute Seperate CLouds method on the rsult of the ransac seperation of the ground plane. returned segResult contains a pc for ground plane and pc from obstacles
  // std::pair<typename pcl::PointCloud<PointT>::Ptr, typename pcl::PointCloud<PointT>::Ptr> segResult = SeparateClouds(inliersPointIndices,cloud);
  // return segResult;
}


template <typename PointT>
// void ProcessPointClouds<PointT>::clusterHelper(int indice, const std::vector<std::vector<float>> points, std::vector<int> &cluster, std::vector<bool> &processed, KdTree_simple* tree, float distanceTol)
// {
//   processed[indice] = true;
//   cluster.push_back(indice);
//   cout << "indice: " << indice << endl;
//   std::vector<int> nearest = tree->search(points[indice], distanceTol);

//   for (int id : nearest)
//   {
//     if (!processed[id])
//       clusterHelper(indice, points, cluster, processed, tree, distanceTol);
//   }
// }

void ProcessPointClouds<PointT>::clusterHelper(int indice, const std::vector<std::vector<float>> points, std::vector<int> &cluster, std::vector<bool> &processed, KdTree_simple *tree, float distanceTol)
{
	processed[indice] = true;
	cluster.push_back(indice);

	std::vector<int> nearest = tree->search(points[indice], distanceTol);

	for (int id : nearest)
	{
		if (!processed[id])
			clusterHelper(id, points, cluster, processed, tree, distanceTol);
	}
}

template <typename PointT>
std::vector<typename pcl::PointCloud<PointT>::Ptr> ProcessPointClouds<PointT>::Clustering(typename pcl::PointCloud<PointT>::Ptr cloud, float clusterTolerance, int minSize, int maxSize)
{

  // Time clustering process
  auto startTime = std::chrono::steady_clock::now();

  std::vector<typename pcl::PointCloud<PointT>::Ptr> clusters;

  // TODO:: Fill in the function to perform euclidean clustering to group detected obstacles

  // need to replace with own KDTree implementation for project
  // TODO, expand the KDTree from the quiz to be 3d. need to cover x,y,z dimensions? i.e mod 3.

  // steps?
  // create tree object
  // add points
  // cluster    	euclideanCluster method returns std::vector<std::vector<int>> clusters
  // might be able to substitue in for the cluster_indicies?

  KdTree_simple *tree_simple = new KdTree_simple;
  std::vector<std::vector<float>> points_test;
	points_test = {{-6.2, 7, 2}, {-6.3, 8.4, 3}, {-5.2, 7.1, 2}, {-5.7, 6.3, 2.5}, {7.2, 6.1, 3}, {8.0, 5.3, 4.1}, {7.2, 7.1, -1}, {0.2, -7.1, -5}, {1.7, -6.9, 2}, {-1.2, -7.2, 3}, {2.2, -8.9, 7}};

  // for (const auto& point : cloud) {
  //     points.push_back(cv::Point3f(point.x, point.y, point.z));
        // points.insert(cv::Point3f(point.x, point.y, point.z),ui);
  // }
  cout << "debugging vector: " << points_test.size() << endl;
  cout << "cloud - clustering input: " << cloud->size() << endl;

  // tree->insert(points[0],0);

  std::vector<std::vector<float>> points_test2;


  for (int i = 0; i < cloud->points.size(); i++)
{

    std::vector<float> vect({cloud->points[i].x, cloud->points[i].y, cloud->points[i].z }); 
    // cout << "cloud points: " << cloud->points[i][0] << "x: " << cloud->points[i].x << endl;

    cout<< "vect: "<< vect[0] <<  "," << vect[1] << "," << vect[2] << endl;
    // points_test.push_back((cloud->points[i].x, cloud->points[i].y, cloud->points[i].z));
    points_test2.push_back(vect);
        // points_test.insert((*vect), i);
}
  cout << "points test size: " << points_test2.size() << endl;
  // TODO: Fill out this function to return list of indices for each cluster

    cout<< "points_test: "<< points_test[0][0] <<  "," << points_test[0][1] << "," << points_test[0][2] << endl;
    // cout<< "points_test2: "<< points_test2[0][0] <<  "," << points_test2[0][1] << "," << points_test2[0][2] << endl;

  for (int i=0; i<points_test2.size(); i++)
  	tree_simple->insert(points_test2[i],i);

  // for (int i=0; i<points_test.size(); i++)
  // 	tree_simple->insert(points_test[i],i);

  std::vector<std::vector<int>> clusters_new;
  std::vector<bool> processed(points_test2.size(), false);

  int i = 0;

  while (i < points_test2.size())
  {
		// cout << "*******: "<< i << endl;
    if (processed[i])
    {
      i++;
      continue;
    }

    std::vector<int> cluster;
    clusterHelper(i, points_test2, cluster, processed, tree_simple, clusterTolerance);
    clusters_new.push_back(cluster);
    i++;
  }
  // points_test2 = NULL;
  // points_test = NULL;
  // clusters_new = NULL;


// TODO convert the  "std::vector<std::vector<int>> clusters_new;"" to the expected return type std::vector<typename pcl::PointCloud<PointT>::Ptr>
// (just because I'm trying to keep the interface the same...)
// clusters is the current returned variable





  // attempt to cluster using the self implemented KdTree
  // clusters_new = tree_simple->euclideanCluster(cloud->points, tree, 3.0);
  cout << "clusters_new size: " << clusters_new.size() << endl;

  pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
  tree->setInputCloud(cloud);

  // pcl::PointIndices pcli = new pcl::PointIndices(1);


  std::vector<pcl::PointIndices> cluster_indices;
  // std::vector<pcl::PointIndices> clusters;
  pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
  ec.setClusterTolerance(clusterTolerance); // 2cm
  ec.setMinClusterSize(minSize);
  ec.setMaxClusterSize(maxSize);
  ec.setSearchMethod(tree);
  ec.setInputCloud(cloud);
  ec.extract(cluster_indices);

  int j = 0;

  // iterates through the indices.
  // I already have indices, they just arent in pcl::PointIndices. mine are in  std::vector<std::vector<int>> clusters_new;

  // iterate through the point cloud
  for (std::vector<std::vector<int>>::const_iterator it = clusters_new.begin(); it != clusters_new.end(); ++it)
  {
    // create a new cloud that represents a cluster
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZ>);

    // iterate through the points idx in the cluster
    for (std::vector<int>::const_iterator pit = it->begin(); pit != it->end(); ++pit)
      cloud_cluster->points.push_back((*cloud)[*pit]);
    cloud_cluster->width = cloud_cluster->points.size();
    cloud_cluster->height = 1;
    cloud_cluster->is_dense = true;

    // std::cout << "PointCloud representing the Cluster: " << cloud_cluster->points.size() << " data points." << std::endl;
    clusters.push_back(cloud_cluster);
    // std::stringstream ss;
    // ss << "cloud_cluster_" << j << ".pcd";
    // writer.write<pcl::PointXYZ> (ss.str (), *cloud_cluster, false); /
    j++;
  }

////////////////////////////////////



  // for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin(); it != cluster_indices.end(); ++it)
  // {
  //   // create a new cloud that represents a cluster
  //   pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZ>);

  //   // iterate through the points idx in the cluster
  //   for (std::vector<int>::const_iterator pit = it->indices.begin(); pit != it->indices.end(); ++pit)
  //     cloud_cluster->points.push_back((*cloud)[*pit]);
  //   cloud_cluster->width = cloud_cluster->points.size();
  //   cloud_cluster->height = 1;
  //   cloud_cluster->is_dense = true;

  //   std::cout << "PointCloud representing the Cluster: " << cloud_cluster->points.size() << " data points." << std::endl;
  //   clusters.push_back(cloud_cluster);
  //   // std::stringstream ss;
  //   // ss << "cloud_cluster_" << j << ".pcd";
  //   // writer.write<pcl::PointXYZ> (ss.str (), *cloud_cluster, false); /
  //   j++;
  // }

  //

  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
  std::cout << "clustering took " << elapsedTime.count() << " milliseconds and found " << clusters.size() << " clusters" << std::endl;


// TODO THERES SOMETHING WRONG HERE. 
// SORT OUT THE RETURN TYPE FROM MY KDTREE AND GET RID OF THE PCI SEARCH KDTREE CODE ABOVE
  return clusters;
}

template <typename PointT>
Box ProcessPointClouds<PointT>::BoundingBox(typename pcl::PointCloud<PointT>::Ptr cluster)
{

  // Find bounding box for one of the clusters
  PointT minPoint, maxPoint;
  pcl::getMinMax3D(*cluster, minPoint, maxPoint);

  Box box;
  box.x_min = minPoint.x;
  box.y_min = minPoint.y;
  box.z_min = minPoint.z;
  box.x_max = maxPoint.x;
  box.y_max = maxPoint.y;
  box.z_max = maxPoint.z;

  return box;
}

template <typename PointT>
void ProcessPointClouds<PointT>::savePcd(typename pcl::PointCloud<PointT>::Ptr cloud, std::string file)
{
  pcl::io::savePCDFileASCII(file, *cloud);
  std::cerr << "Saved " << cloud->points.size() << " data points to " + file << std::endl;
}

template <typename PointT>
typename pcl::PointCloud<PointT>::Ptr ProcessPointClouds<PointT>::loadPcd(std::string file)
{

  typename pcl::PointCloud<PointT>::Ptr cloud(new pcl::PointCloud<PointT>);

  if (pcl::io::loadPCDFile<PointT>(file, *cloud) == -1) //* load the file
  {
    PCL_ERROR("Couldn't read file \n");
  }
  std::cerr << "Loaded " << cloud->points.size() << " data points from " + file << std::endl;

  return cloud;
}

template <typename PointT>
std::vector<boost::filesystem::path> ProcessPointClouds<PointT>::streamPcd(std::string dataPath)
{

  std::vector<boost::filesystem::path> paths(boost::filesystem::directory_iterator{dataPath}, boost::filesystem::directory_iterator{});

  // sort files in accending order so playback is chronological
  sort(paths.begin(), paths.end());

  return paths;
}
