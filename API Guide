# (Unofficial) guide to AP's API
Please report any missing features or changes to the API.

## Updating Anime List
### Anime Status
`https://www.anime-planet.com/api/list/status/anime/ANIME_ID/[0-6]/SESSION_ID`
ANIME_ID=Anime's specific ID.
SESSION_ID=session specific hash used to validate requests, it is still nessecary to be logged in to make requests

0=Blank/Not on list
1=Finished
2=Watching
3=Dropped
4=Want to Watch
5=Stalled (not impleneted on ths program)
6=Won't Watch (not impleneted on ths program)

### Episode Status
`https://www.anime-planet.com/api/list/update/anime/ANIME_ID/CURRENT_EPISODE/SESSION_ID`
ANIME_ID=Anime's specific ID.
SESSION_ID=session specific hash used to validate requests, it is still nessecary to be logged in to make requests
CURRENT_EPISODE=Episode currently watching, giving an episode bigger than the number episodes listed fails.

### Anime Rating
`https://www.anime-planet.com/api/list/rate/anime/ANIME_ID/RATING/SESSION_ID`
ANIME_ID=Anime's specific ID.
SESSION_ID=session specific hash used to validate requests, it is still nessecary to be logged in to make requests
RATING=0-5 with .5 increments (eg. 0, 0.5, 1, 1.5, etc.)
