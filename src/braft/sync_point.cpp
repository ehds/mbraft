// Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include "sync_point.h"

#include <fcntl.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#ifndef NDEBUG
namespace braft {

struct SyncPoint::Data {
    Data() : enabled_(false) {}
    // Enable proper deletion by subclasses
    virtual ~Data() {}
    // successor/predecessor map loaded from LoadDependency
    std::unordered_map<std::string, std::vector<std::string>> successors_;
    std::unordered_map<std::string, std::vector<std::string>> predecessors_;
    std::unordered_map<std::string, std::function<void(void*)>> callbacks_;
    std::unordered_map<std::string, std::vector<std::string>> markers_;
    std::unordered_map<std::string, std::thread::id> marked_thread_id_;

    std::mutex mutex_;
    std::condition_variable cv_;
    // sync points that have been passed through
    std::unordered_set<std::string> cleared_points_;
    std::atomic<bool> enabled_;
    int num_callbacks_running_ = 0;

    void LoadDependency(const std::vector<SyncPointPair>& dependencies);
    void LoadDependencyAndMarkers(
        const std::vector<SyncPointPair>& dependencies,
        const std::vector<SyncPointPair>& markers);
    bool PredecessorsAllCleared(const std::string& point);
    void SetCallBack(const std::string& point,
                     const std::function<void(void*)>& callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_[point] = callback;
    }

    void ClearCallBack(const std::string& point);
    void ClearAllCallBacks();
    void EnableProcessing() { enabled_ = true; }
    void DisableProcessing() { enabled_ = false; }
    void ClearTrace() {
        std::lock_guard<std::mutex> lock(mutex_);
        cleared_points_.clear();
    }
    bool DisabledByMarker(const std::string& point, std::thread::id thread_id) {
        auto marked_point_iter = marked_thread_id_.find(point);
        return marked_point_iter != marked_thread_id_.end() &&
               thread_id != marked_point_iter->second;
    }
    void Process(const std::string& point, void* cb_arg);
};

SyncPoint* SyncPoint::GetInstance() {
    static SyncPoint sync_point;
    return &sync_point;
}

SyncPoint::SyncPoint() : impl_(new Data) {}

SyncPoint::~SyncPoint() { delete impl_; }

void SyncPoint::LoadDependency(const std::vector<SyncPointPair>& dependencies) {
    impl_->LoadDependency(dependencies);
}

void SyncPoint::LoadDependencyAndMarkers(
    const std::vector<SyncPointPair>& dependencies,
    const std::vector<SyncPointPair>& markers) {
    impl_->LoadDependencyAndMarkers(dependencies, markers);
}

void SyncPoint::SetCallBack(const std::string& point,
                            const std::function<void(void*)>& callback) {
    impl_->SetCallBack(point, callback);
}

void SyncPoint::ClearCallBack(const std::string& point) {
    impl_->ClearCallBack(point);
}

void SyncPoint::ClearAllCallBacks() { impl_->ClearAllCallBacks(); }

void SyncPoint::EnableProcessing() { impl_->EnableProcessing(); }

void SyncPoint::DisableProcessing() { impl_->DisableProcessing(); }

void SyncPoint::ClearTrace() { impl_->ClearTrace(); }

void SyncPoint::Process(const std::string& point, void* cb_arg) {
    impl_->Process(point, cb_arg);
}

void SyncPoint::Data::LoadDependency(
    const std::vector<SyncPointPair>& dependencies) {
    std::lock_guard<std::mutex> lock(mutex_);
    successors_.clear();
    predecessors_.clear();
    cleared_points_.clear();
    for (const auto& dependency : dependencies) {
        successors_[dependency.predecessor].push_back(dependency.successor);
        predecessors_[dependency.successor].push_back(dependency.predecessor);
    }
    cv_.notify_all();
}

void SyncPoint::Data::LoadDependencyAndMarkers(
    const std::vector<SyncPointPair>& dependencies,
    const std::vector<SyncPointPair>& markers) {
    std::lock_guard<std::mutex> lock(mutex_);
    successors_.clear();
    predecessors_.clear();
    cleared_points_.clear();
    markers_.clear();
    marked_thread_id_.clear();
    for (const auto& dependency : dependencies) {
        successors_[dependency.predecessor].push_back(dependency.successor);
        predecessors_[dependency.successor].push_back(dependency.predecessor);
    }
    for (const auto& marker : markers) {
        successors_[marker.predecessor].push_back(marker.successor);
        predecessors_[marker.successor].push_back(marker.predecessor);
        markers_[marker.predecessor].push_back(marker.successor);
    }
    cv_.notify_all();
}

bool SyncPoint::Data::PredecessorsAllCleared(const std::string& point) {
    for (const auto& pred : predecessors_[point]) {
        if (cleared_points_.count(pred) == 0) {
            return false;
        }
    }
    return true;
}

void SyncPoint::Data::ClearCallBack(const std::string& point) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (num_callbacks_running_ > 0) {
        cv_.wait(lock);
    }
    callbacks_.erase(point);
}

void SyncPoint::Data::ClearAllCallBacks() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (num_callbacks_running_ > 0) {
        cv_.wait(lock);
    }
    callbacks_.clear();
}

void SyncPoint::Data::Process(const std::string& point, void* cb_arg) {
    if (!enabled_) {
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto thread_id = std::this_thread::get_id();

    auto marker_iter = markers_.find(point);
    if (marker_iter != markers_.end()) {
        for (auto& marked_point : marker_iter->second) {
            marked_thread_id_.emplace(marked_point, thread_id);
        }
    }

    if (DisabledByMarker(point, thread_id)) {
        return;
    }

    while (!PredecessorsAllCleared(point)) {
        cv_.wait(lock);
        if (DisabledByMarker(point, thread_id)) {
            return;
        }
    }

    auto callback_pair = callbacks_.find(point);
    if (callback_pair != callbacks_.end()) {
        num_callbacks_running_++;
        mutex_.unlock();
        callback_pair->second(cb_arg);
        mutex_.lock();
        num_callbacks_running_--;
    }
    cleared_points_.insert(point);
    cv_.notify_all();
}
}  // namespace braft
#endif  // NDEBUG
