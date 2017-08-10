#include <vector>

#include "caffe/layers/jmmd_layer.hpp"

namespace caffe {

template <typename Dtype>
void JMMDLossLayer<Dtype>::LayerSetUp(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::LayerSetUp(bottom, top);
  dim_ = bottom[0]->count() / bottom[0]->count(0, 1);
  source_num_ = bottom[0]->count(0, 1);
  target_num_ = bottom[1]->count(0, 1);
  total_num_ = source_num_ + target_num_;
  kernel_num_ = this->layer_param_.jmmd_param().kernel_num(); 
  label_kernel_num_ = this->layer_param_.jmmd_param().label_kernel_num(); 
  fix_gamma_ = this->layer_param_.jmmd_param().fix_gamma();
  sigma_ = this->layer_param_.jmmd_param().sigma();
  auto_sigma_ = this->layer_param_.jmmd_param().auto_sigma();
  label_back_propagate_ = this->layer_param_.jmmd_param().label_back_propagate();
  if(label_back_propagate_){
      if(bottom.size() < 5){
          LOG(FATAL) << "Not enough bottom for label back propagate";
      }
  }
  gamma_ = Dtype(-1);
  kernel_mul_ = this->layer_param_.jmmd_param().kernel_mul();
  label_kernel_mul_ = this->layer_param_.jmmd_param().label_kernel_mul();
  diff_.Reshape(1, total_num_, total_num_, dim_);
  for(int i = 0;i < kernel_num_;++i){
      Blob<Dtype>* temp = new Blob<Dtype>(1, 1, total_num_, total_num_);
      kernel_val_.push_back(temp);
  }
  diff_multiplier_.Reshape(1, 1, 1, dim_);
  delta_.Reshape(1, 1, total_num_, total_num_);
  caffe_set(dim_, Dtype(1), diff_multiplier_.mutable_cpu_data());
  loss_weight_ = this->layer_param_.loss_weight(0);
  label_loss_weight_ = this->layer_param_.loss_weight(1);
  label_kernel_mode_ = this->layer_param_.jmmd_param().label_kernel_mode();
}

template <typename Dtype>
void JMMDLossLayer<Dtype>::Reshape(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  vector<int> loss_shape(0);
  top[0]->Reshape(loss_shape);
  if(top.size() == 2){
      top[1]->Reshape(loss_shape);
  }
  dim_ = bottom[0]->count() / bottom[0]->count(0, 1);
  source_num_ = bottom[0]->count(0, 1);
  target_num_ = bottom[1]->count(0, 1);
  total_num_ = source_num_ + target_num_;
  diff_.Reshape(1, total_num_, total_num_, dim_);
  for(int i = 0;i < kernel_num_;++i){
    kernel_val_[i]->Reshape(1, 1, total_num_, total_num_);
  }
  diff_multiplier_.Reshape(1, 1, 1, dim_);
  caffe_set(dim_, Dtype(1), diff_multiplier_.mutable_cpu_data());
  delta_.Reshape(1, 1, total_num_, total_num_);
}

template <typename Dtype>
void JMMDLossLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
}

/*template <typename Dtype>*/
//Dtype MMDLossLayer<Dtype>::get_label_kernel(MMDParameter_LabelKernelMode kernel_mode, 
        //int source_label, const Dtype* target_p, const int label_dim){
  //Dtype kernel_val = Dtype(0);
  //if(kernel_mode == MMDParameter_LabelKernelMode_RBF){
      //Dtype sum = 0;
      //for(int i = 0;i < label_dim;++i){
          //sum += (i == source_label) ? 
              //(Dtype(1) - target_p[i]) * (Dtype(1) - target_p[i]):
              //target_p[i] * target_p[i];
      //}
      //kernel_val = exp(-sum / sigma_);
  //}
  //else if(kernel_mode == MMDParameter_LabelKernelMode_IDENTIFY){
      //Dtype max = target_p[source_label];
      //kernel_val = Dtype(1);
      //for(int i = 0;i < label_dim;++i){
          //if(target_p[i] > max){
              //kernel_val = Dtype(0);
              //break;
          //}
      //}
  //}
  //else{
      //LOG(FATAL) << "Unknown label kernel mode: "
          //<< MMDParameter_LabelKernelMode_Name(kernel_mode);
  //}
  //return kernel_val;
//}

//template <typename Dtype>
//Dtype MMDLossLayer<Dtype>::get_label_kernel(MMDParameter_LabelKernelMode kernel_mode, 
        //const Dtype* target_p1, const Dtype* target_p2, const int label_dim){
  //Dtype kernel_val = Dtype(0);
  //if(kernel_mode == MMDParameter_LabelKernelMode_RBF){
      //Dtype sum = 0;
      //for(int i = 0;i < label_dim;++i){
          //sum += (target_p1[i] - target_p2[i]) * (target_p1[i] - target_p2[i]);
      //}
      //kernel_val = exp(-sum / sigma_);
  //}
  //else if(kernel_mode == MMDParameter_LabelKernelMode_IDENTIFY){
      //Dtype max = Dtype(-1);
      //int max_index;
      //for(int i = 0;i < label_dim;++i){
          //if(target_p1[i] > max){
              //max = target_p1[i];
              //max_index = i;
          //}
      //}
      //kernel_val = Dtype(1);
      //max = target_p2[max_index];
      //for(int i = 0;i < label_dim;++i){
          //if(target_p2[i] > max){
              //kernel_val = Dtype(0);
              //break;
          //}
      //}
  //}
  //else{
      //LOG(FATAL) << "Unknown label kernel mode: "
          //<< MMDParameter_LabelKernelMode_Name(kernel_mode);
  //}
  //return kernel_val;
//}

//template <typename Dtype>
//Dtype MMDLossLayer<Dtype>::get_label_kernel(MMDParameter_LabelKernelMode kernel_mode, 
        //const int source_label1, const int source_label2){
  //Dtype kernel_val = Dtype(0);
  //if(kernel_mode == MMDParameter_LabelKernelMode_RBF){
      //kernel_val = (source_label1 == source_label2) ? Dtype(1) : exp(Dtype(-1));
  //}
  //else if(kernel_mode == MMDParameter_LabelKernelMode_IDENTIFY){
      //kernel_val = (source_label1 == source_label2) ? Dtype(1) : Dtype(0);
  //}
  //else{
      //LOG(FATAL) << "Unknown label kernel mode: "
          //<< MMDParameter_LabelKernelMode_Name(kernel_mode);
  //}
  //return kernel_val;
/*}*/

template <typename Dtype>
void JMMDLossLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down,
    const vector<Blob<Dtype>*>& bottom) {
}

#ifdef CPU_ONLY
STUB_GPU(JMMDLossLayer);
#endif

INSTANTIATE_CLASS(JMMDLossLayer);
REGISTER_LAYER_CLASS(JMMDLoss);

}  // namespace caffe
