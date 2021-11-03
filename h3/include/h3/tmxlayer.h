#ifndef _H3_TMXLAYER_H_
#define _H3_TMXLAYER_H_

#include <tmxlite/Map.hpp>

#include <memory>

#define SFML_STATIC
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/detail/Log.hpp>

using H3TextureResource   = std::map<std::string, std::unique_ptr<sf::Texture>>;
using H3TmxLayerChunkTile = std::array<sf::Vertex, 4u>;

struct SH3TmxAnimationState
{
    sf::Vector2u       tileCords;
    sf::Time           startTime;
    sf::Time           currentTime;
    tmx::Tileset::Tile animTile;
    std::uint8_t       flipFlags;
};

class CH3TmxLayerChunkArray final
    : public sf::Drawable
{
    const sf::Texture& m_texture;
    std::vector<sf::Vertex> m_vertices;
    void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

public:
    using Ptr = std::unique_ptr<CH3TmxLayerChunkArray>;

    tmx::Vector2u tileSetSize;
    sf::Vector2u  tsTileCount;
    std::uint32_t m_firstGID,
                  m_lastGID;

    explicit CH3TmxLayerChunkArray(const sf::Texture& t, const tmx::Tileset& ts);
    CH3TmxLayerChunkArray(const CH3TmxLayerChunkArray&) = delete;
    ~CH3TmxLayerChunkArray() = default;

    CH3TmxLayerChunkArray& operator=(const CH3TmxLayerChunkArray&) = delete;

    void reset();
    void addTile(const H3TmxLayerChunkTile& tile);
    sf::Vector2u getTextureSize() const;
};

class CH3TmxLayerChunk final
    : public sf::Transformable
    , public sf::Drawable
{
public:
    using Ptr = std::unique_ptr<CH3TmxLayerChunk>;
    using Tile = std::array<sf::Vertex, 4u>;

private:
    sf::Uint8                                   layerOpacity;       // opacity of the layer
    sf::Vector2f                                layerOffset;        // Layer offset
    sf::Vector2u                                mapTileSize;        // general Tilesize of Map
    sf::Vector2f                                chunkTileCount;     // chunk tilecount
    std::vector<tmx::TileLayer::Tile>           m_chunkTileIDs;     // stores all tiles in this chunk for later manipulation
    std::vector<sf::Color>                      m_chunkColors;      // stores colors for extended color effects
    std::map<std::uint32_t, tmx::Tileset::Tile> m_animTiles;        // animation catalogue
    std::vector<SH3TmxAnimationState>           m_activeAnimations; // Animations to be done in this chunk
    std::vector<CH3TmxLayerChunkArray::Ptr>     m_chunkArrays;

    void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

public:
    CH3TmxLayerChunk(const tmx::TileLayer& layer, std::vector<const tmx::Tileset*> tilesets,
                     const sf::Vector2f& position, const sf::Vector2f& tileCount, const sf::Vector2u& tileSize,
                     std::size_t rowSize, H3TextureResource& tr, const std::map<std::uint32_t, tmx::Tileset::Tile>& animTiles);
    CH3TmxLayerChunk(const CH3TmxLayerChunk&) = delete;
    ~CH3TmxLayerChunk() = default;

    CH3TmxLayerChunk& operator=(const CH3TmxLayerChunk&) = delete;

    std::vector<SH3TmxAnimationState>& getActiveAnimations() { return m_activeAnimations; }

    void                 generateTiles(bool registerAnimation = false);
    tmx::TileLayer::Tile getTile(int x, int y) const;
    void                 setTile(int x, int y, tmx::TileLayer::Tile tile, bool refresh);
    sf::Color            getColor(int x, int y) const;
    void                 setColor(int x, int y, sf::Color color, bool refresh);
    void                 maybeRegenerate(bool refresh);
    int                  calcIndexFrom(int x, int y) const;
    bool                 empty() const;

    void flipY(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3);
    void flipX(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3);
    void flipD(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3);
    void doFlips(std::uint8_t bits, sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3);
};

class CH3TmxLayer final
    : public sf::Drawable
{
    //increasing m_chunkSize by 4; fixes render problems when mapsize != chunksize
    //sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
    sf::Vector2f m_chunkSize = sf::Vector2f(512.f, 512.f);
    sf::Vector2u m_chunkCount;
    sf::Vector2u m_MapTileSize;   // general Tilesize of Map
    sf::FloatRect m_globalBounds;

    H3TextureResource m_textureResource;

    std::vector<CH3TmxLayerChunk::Ptr> m_chunks;
    mutable std::vector<CH3TmxLayerChunk*> m_visibleChunks;
    CH3TmxLayerChunk::Ptr& getChunkAndTransform(int x, int y, sf::Vector2u& chunkRelative);
    void createChunks(const tmx::Map& map, const tmx::TileLayer& layer);

    void updateVisibility(const sf::View& view) const;

    void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

public:
    CH3TmxLayer(const tmx::Map& map, size_t idx);

    ~CH3TmxLayer() = default;
    CH3TmxLayer(const CH3TmxLayer&) = delete;
    CH3TmxLayer& operator = (const CH3TmxLayer&) = delete;

    const sf::FloatRect& getGlobalBounds() const;

    void setTile(int tileX, int tileY, tmx::TileLayer::Tile tile, bool refresh = true);

    tmx::TileLayer::Tile getTile(int tileX, int tileY);
    void setColor(int tileX, int tileY, sf::Color color, bool refresh = true);

    sf::Color getColor(int tileX, int tileY);

    void update(sf::Time elapsed);
};

#endif /* _H3_TMXLAYER_H_ */
