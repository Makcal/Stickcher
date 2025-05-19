CREATE TABLE IF NOT EXISTS sticker (
    file_unique_id TEXT PRIMARY KEY,
    file_id TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS sticker_file_id_idx ON sticker
    USING hash (file_id);

CREATE TABLE IF NOT EXISTS sticker_pack (
    id UUID PRIMARY KEY,
    name TEXT NOT NULL,
    owner_id BIGINT NOT NULL
);
CREATE INDEX IF NOT EXISTS sticker_pack_owner_id_idx ON sticker_pack
    USING btree (owner_id);

CREATE TABLE IF NOT EXISTS tag (
    text TEXT NOT NULL,
    sticker_id TEXT NOT NULL,
    pack_id UUID NOT NULL,
    FOREIGN KEY (sticker_id) REFERENCES sticker(file_unique_id)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY (pack_id) REFERENCES sticker_pack(id)
);
CREATE INDEX IF NOT EXISTS tag_pack_id_text_sticker_id_idx ON tag
    USING btree (pack_id) INCLUDE (text, sticker_id);
