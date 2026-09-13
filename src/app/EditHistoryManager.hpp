#pragma once

#include "model/Project.hpp"

#include <string>
#include <utility>
#include <vector>

namespace app {

class EditHistoryManager {
public:
    explicit EditHistoryManager(size_t maxEntries = 200) : maxHistoryEntries(maxEntries) {}

    void Clear() {
        undoStack.clear();
        redoStack.clear();
        isBatchOpen = false;
        batchChanged = false;
        batchLabel.clear();
        batchBefore = Model::Project{};
    }

    void BeginBatch(const Model::Project& project, const std::string& label) {
        if (isBatchOpen) {
            return;
        }

        isBatchOpen = true;
        batchChanged = false;
        batchLabel = label;
        batchBefore = project;
    }

    void MarkChanged() {
        if (isBatchOpen) {
            batchChanged = true;
        }
    }

    bool EndBatch(Model::Project& project) {
        if (!isBatchOpen) {
            return false;
        }

        const bool shouldCommit = batchChanged;
        if (shouldCommit) {
            undoStack.push_back(batchBefore);
            if (undoStack.size() > maxHistoryEntries) {
                undoStack.erase(undoStack.begin());
            }
            redoStack.clear();
        }

        isBatchOpen = false;
        batchChanged = false;
        batchLabel.clear();
        batchBefore = Model::Project{};
        return shouldCommit;
    }

    template <typename TFunc>
    void Execute(Model::Project& project, const std::string& label, TFunc&& func) {
        BeginBatch(project, label);
        func(project);
        MarkChanged();
        EndBatch(project);
    }

    template <typename TFunc>
    bool ApplyInBatch(Model::Project& project, const std::string& label, TFunc&& func) {
        if (!isBatchOpen) {
            BeginBatch(project, label);
        }

        func(project);
        MarkChanged();
        return true;
    }

    bool Undo(Model::Project& project) {
        if (isBatchOpen) {
            EndBatch(project);
        }

        if (undoStack.empty()) {
            return false;
        }

        redoStack.push_back(project);
        project = undoStack.back();
        undoStack.pop_back();
        return true;
    }

    bool Redo(Model::Project& project) {
        if (isBatchOpen) {
            EndBatch(project);
        }

        if (redoStack.empty()) {
            return false;
        }

        undoStack.push_back(project);
        project = redoStack.back();
        redoStack.pop_back();
        return true;
    }

    bool CanUndo() const {
        return !undoStack.empty();
    }

    bool CanRedo() const {
        return !redoStack.empty();
    }

    bool IsBatchOpen() const {
        return isBatchOpen;
    }

private:
    size_t maxHistoryEntries;

    bool isBatchOpen{false};
    bool batchChanged{false};
    std::string batchLabel;
    Model::Project batchBefore;

    std::vector<Model::Project> undoStack;
    std::vector<Model::Project> redoStack;
};

} // namespace app
