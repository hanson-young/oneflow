/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <cstdint>
#include "oneflow/core/framework/framework.h"

namespace oneflow {

REGISTER_USER_OP("generate_random_batch_permutation_indices")
    .Input("x")
    .Output("y")
    .Attr("seed", UserOpAttrType::kAtInt64)
    .SetTensorDescInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      *ctx->Shape4ArgNameAndIndex("y", 0) = Shape({ctx->Shape4ArgNameAndIndex("x", 0)->At(0)});
      *ctx->Dtype4ArgNameAndIndex("y", 0) = DataType::kInt32;
      return Maybe<void>::Ok();
    })
    .SetBatchAxisInferFn([](user_op::BatchAxisContext* ctx) -> Maybe<void> {
      if (ctx->BatchAxis4ArgNameAndIndex("x", 0)->has_value()
          && ctx->BatchAxis4ArgNameAndIndex("x", 0)->value() == 0) {
        ctx->BatchAxis4ArgNameAndIndex("y", 0)->set_value(0);
      } else {
        ctx->BatchAxis4ArgNameAndIndex("y", 0)->clear_value();
      }
      return Maybe<void>::Ok();
    })
    .SetGetSbpFn([](user_op::SbpContext* ctx) -> Maybe<void> {
      ctx->NewBuilder()
          .PartialSum(user_op::OpArg("x", 0))
          .Broadcast(user_op::OpArg("y", 0))
          .Build();
      const auto& x_tensor = ctx->LogicalTensorDesc4InputArgNameAndIndex("x", 0);
      FOR_RANGE(int64_t, i, 0, x_tensor.shape().NumAxes()) {
        ctx->NewBuilder()
            .Split(user_op::OpArg("x", 0), i)
            .Broadcast(user_op::OpArg("y", 0))
            .Build();
      }
      return Maybe<void>::Ok();
    })
    .SetInputArgModifyFn([](user_op::GetInputArgModifier GetInputArgModifierFn,
                            const user_op::UserOpConfWrapper&) {
      GetInputArgModifierFn("x", 0)->set_use_header_only(true);
    });

}  // namespace oneflow
