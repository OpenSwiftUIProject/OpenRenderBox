import ProjectDescription

let tuist = Tuist(
    project: .tuist(
        generationOptions: .options(
            optionalAuthentication: true,
            enableCaching: false
        ),
        cacheOptions: .options(storages: [.local])
    )
)
