#pragma once

#include "types.hpp"

#include <tg_stater/state_storage/common.hpp>
#include <tg_stater/state_storage/memory.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace states {

using namespace db::models;

namespace detail {

struct PackIdMixin {
    StickerPackId packId;
    PackIdMixin(StickerPackId packId) : packId{packId} {} // NOLINT(*-explicit-*)
};

} // namespace detail

struct PackList {};

struct PackCreateEnterName {};

struct PackImportEnterName {};

struct PackView : detail::PackIdMixin {};

struct PackDeletion : detail::PackIdMixin {};

struct StickerDeletion : detail::PackIdMixin {};

struct StickerAddition : detail::PackIdMixin {};

struct TagAddition : detail::PackIdMixin {
    StickerFileId stickerFileId;
    StickerFileUniqueId stickerFileUniqueId;
    bool hasParsedTag = false;
    std::vector<std::string> tags;

    TagAddition(StickerPackId packId, std::string_view fileId, std::string_view fileUniqueId)
        : PackIdMixin{packId}, stickerFileId{fileId}, stickerFileUniqueId{fileUniqueId} {}

    TagAddition(StickerPackId packId, std::string_view fileId, std::string_view fileUniqueId, std::string_view firstTag)
        : PackIdMixin{packId}, stickerFileId{fileId}, stickerFileUniqueId{fileUniqueId}, hasParsedTag{true} {
        tags.emplace_back(firstTag);
    }

    TagAddition(StickerPackId packId,
                std::string_view fileId,
                std::string_view fileUniqueId,
                std::vector<std::string> tags)
        : PackIdMixin{packId}, stickerFileId{fileId}, stickerFileUniqueId{fileUniqueId}, tags{std::move(tags)} {}
};

struct EditorList : detail::PackIdMixin {};

using State = std::variant<PackList,
                           PackCreateEnterName,
                           PackImportEnterName,
                           PackView,
                           PackDeletion,
                           StickerDeletion,
                           StickerAddition,
                           TagAddition,
                           EditorList>;

using StateManager = tg_stater::StateProxy<tg_stater::MemoryStateStorage<State>>;

} // namespace states
