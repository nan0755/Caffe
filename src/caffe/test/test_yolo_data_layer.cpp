// Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved
// @author erlangz(zhengwenchao@baidu.com)
// @date 2017/03/13 13:38:23
// @file src/caffe/test/test_yolo_data_layer.cpp
// @brief 
// 
#include <cmath>
#include <vector>

#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/yolo_data_layer.hpp"
#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"

namespace caffe {

template <typename TypeParam>
class YoloDataLayerTest : public MultiDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;

 protected:
  YoloDataLayerTest<TypeParam>() {

  }
  virtual ~YoloDataLayerTest<TypeParam>() {

  }
};

TYPED_TEST_CASE(YoloDataLayerTest, TestDtypesAndDevices);

TYPED_TEST(YoloDataLayerTest, TestSetup) {

}

TYPED_TEST(YoloDataLayerTest, TestReshape) {
   typedef typename TypeParam::Dtype Dtype;
   LayerParameter layer_param;
   layer_param.mutable_data_param()->set_batch_size(16); 
   std::string db_file(CMAKE_SOURCE_DIR "/caffe/test/test_data/yolo_train_lmdb");

   layer_param.mutable_data_param()->set_source(db_file);
   layer_param.mutable_data_param()->set_backend(DataParameter::LMDB);
   layer_param.mutable_yolo_data_param()->set_max_labels_number(2); //2 * 5 + 1
   YoloDataLayer<Dtype> yolo_data_layer(layer_param);
   
   vector<Blob<Dtype>*> bottom;
   vector<Blob<Dtype>*> top(2, NULL);
   top[0] = new Blob<Dtype>();
   top[1] = new Blob<Dtype>();

   yolo_data_layer.LayerSetUp(bottom, top);
   for (int i = 0; i < BasePrefetchingDataLayer<Dtype>::PREFETCH_COUNT; i++) {
       EXPECT_EQ(16, yolo_data_layer.prefetch_[i].data_.num());
       EXPECT_EQ(3, yolo_data_layer.prefetch_[i].data_.channels());
       EXPECT_EQ(448, yolo_data_layer.prefetch_[i].data_.height());
       EXPECT_EQ(448, yolo_data_layer.prefetch_[i].data_.width());

       EXPECT_EQ(16, yolo_data_layer.prefetch_[i].label_.num());
       EXPECT_EQ(11, yolo_data_layer.prefetch_[i].label_.channels());
       EXPECT_EQ(1, yolo_data_layer.prefetch_[i].label_.height());
       EXPECT_EQ(1, yolo_data_layer.prefetch_[i].label_.width());
   }
   delete top[0];
   delete top[1];
}

TYPED_TEST(YoloDataLayerTest, TestBatchLoad) {
   typedef typename TypeParam::Dtype Dtype;
   LayerParameter layer_param;
   layer_param.mutable_data_param()->set_batch_size(4); 
   std::string db_file(CMAKE_SOURCE_DIR "/caffe/test/test_data/yolo_train_lmdb");

   layer_param.mutable_data_param()->set_source(db_file);
   layer_param.mutable_data_param()->set_backend(DataParameter::LMDB);
   layer_param.mutable_yolo_data_param()->set_max_labels_number(10); //10 * 5 + 1
   YoloDataLayer<Dtype> yolo_data_layer(layer_param);
   
   vector<Blob<Dtype>*> bottom;
   vector<Blob<Dtype>*> top(2, NULL);
   top[0] = new Blob<Dtype>();
   top[1] = new Blob<Dtype>();
   yolo_data_layer.LayerSetUp(bottom, top);
   yolo_data_layer.Forward(bottom, top);

   EXPECT_EQ(4, top[0]->num()); 
   EXPECT_EQ(3, top[0]->channels()); 
   EXPECT_EQ(448, top[0]->height()); 
   EXPECT_EQ(448, top[0]->width()); 
   
   EXPECT_EQ(4, top[1]->num());
   EXPECT_EQ(51, top[1]->channels()); 
   EXPECT_EQ(1, top[1]->height()); 
   EXPECT_EQ(1, top[1]->width()); 

   const Dtype* label = top[1]->cpu_data();
   bool found = false;
   for (int i = 0; i < top[1]->num(); i++) {
       if (static_cast<int>(label[0]) == 7) {
           found = true;
           //2008_002491.jpg 
           EXPECT_NEAR(17.0, label[1], 1e-6); //person
           EXPECT_NEAR(0.60900, label[2], 1e-6);
           EXPECT_NEAR(0.735735, label[3], 1e-6);
           EXPECT_NEAR(0.63400, label[4], 1e-6);
           EXPECT_NEAR(0.528528, label[5], 1e-6);
       }
       label += top[1]->count(1);
   }
   EXPECT_TRUE(found);
   delete top[0];
   delete top[1];
}
}
