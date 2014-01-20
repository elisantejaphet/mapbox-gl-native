#include <llmr/map/tile.hpp>

#include <stdint.h>
#include <cassert>
// #include <iostream>
#include <thread>

#include <llmr/util/pbf.hpp>
#include <llmr/util/string.hpp>
#include <llmr/geometry/geometry.hpp>
#include <cmath>

using namespace llmr;

Tile::ID Tile::parent(const ID& id, int32_t z) {
    assert(z < id.z);
    ID pos(id);
    while (pos.z > z) {
        pos.z--;
        pos.x = floor(pos.x / 2);
        pos.y = floor(pos.y / 2);
    }
    return pos;
}


std::forward_list<Tile::ID> Tile::children(const ID& id, int32_t z) {
    assert(z > id.z);
    int32_t factor = pow(2, z - id.z);

    std::forward_list<ID> children;
    for (int32_t y = id.y * factor, y_max = (id.y + 1) * factor; y < y_max; y++) {
        for (int32_t x = id.x * factor, x_max = (id.x + 1) * factor; x < x_max; x++) {
            children.emplace_front(x, y, z);
        }
    }
    return children;
}


Tile::Tile(ID id)
    : id(id),
      state(initial),
      data(0),
      bytes(0) {

    // Initialize tile debug coordinates
    char coord[32];
    snprintf(coord, sizeof(coord), "%d/%d/%d", id.z, id.x, id.y);
    debugFontVertex.addText(coord, 50, 200, 5);
}

Tile::~Tile() {
    // fprintf(stderr, "[%p] deleting tile %d/%d/%d\n", this, id.z, id.x, id.y);
    if (this->data) {
        free(this->data);
    }
}

const std::string Tile::toString() const {
    return util::sprintf("[tile %d/%d/%d]", id.z, id.x, id.y);
}


void Tile::setData(uint8_t *data, uint32_t bytes) {
    this->data = (uint8_t *)malloc(bytes);
    this->bytes = bytes;
    memcpy(this->data, data, bytes);
}

void Tile::cancel() {
    // TODO: thread safety
    if (state != obsolete) {
        state = obsolete;
    } else {
        assert((!"logic error? multiple cancelleations"));
    }
}

bool Tile::parse() {
    if (state == obsolete) {
        return false;
    }

    // fprintf(stderr, "[%p] parsing tile [%d/%d/%d]...\n", this, z, x, y);

    pbf tile(data, bytes);

    int code = setjmp(tile.jump_buffer);
    if (code > 0) {
        fprintf(stderr, "[%p] parsing tile [%d/%d/%d]... failed: %s\n", this, id.z, id.x, id.y, tile.msg.c_str());
        cancel();
        return false;
    }

    while (tile.next()) {
        if (tile.tag == 3) { // layer
            uint32_t bytes = (uint32_t)tile.varint();
            parseLayer(tile.data, bytes);
            tile.skipBytes(bytes);
        } else {
            tile.skip();
        }
    }

    if (state == obsolete) {
        return false;
    } else {
        state = ready;
    }

    return true;
}

void Tile::parseLayer(const uint8_t *data, uint32_t bytes) {
    pbf layer(data, bytes);
    std::string name;
    while (layer.next()) {
        if (layer.tag == 1) {
            name = layer.string();
        } else if (layer.tag == 2) {
            uint32_t bytes = (uint32_t)layer.varint();
            parseFeature(layer.data, bytes);
            layer.skipBytes(bytes);
        } else {
            layer.skip();
        }
    }
}

void Tile::parseFeature(const uint8_t *data, uint32_t bytes) {
    pbf feature(data, bytes);
    while (feature.next()) {
        if (feature.tag == 1) {
            /*uint32_t id =*/ feature.varint();
        } else if (feature.tag == 2) {
            const uint8_t *tag_end = feature.data + feature.varint();
            while (feature.data < tag_end) {
                /*uint32_t key =*/ feature.varint();
                /*uint32_t value =*/ feature.varint();
            }
        } else if (feature.tag == 3) {
            /*uint32_t type =*/ feature.varint();
        } else if (feature.tag == 4) {
            uint32_t bytes = (uint32_t)feature.varint();
            loadGeometry(feature.data, bytes);
            feature.skipBytes(bytes);
        } else {
            feature.skip();
        }
    }
}

void Tile::loadGeometry(const uint8_t *data, uint32_t bytes) {
    geometry geometry(data, bytes);

    geometry::command cmd;
    int32_t x, y;
    while ((cmd = geometry.next(x, y)) != geometry::end) {
        if (cmd == geometry::move_to) {
            lineVertex.addDegenerate();
        }

        lineVertex.addCoordinate(x, y);
    }
}
