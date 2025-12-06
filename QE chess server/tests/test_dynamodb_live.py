import boto3
import pytest
import uuid


TABLE_NAME = "Flock-Chess"


@pytest.fixture
def dynamodb_table():
    dynamodb = boto3.resource("dynamodb")
    return dynamodb.Table(TABLE_NAME)


def test_put_get_delete_item(dynamodb_table):
    table = dynamodb_table

    # Generate a unique test record
    sub = "test-user-" + str(uuid.uuid4())
    game_id = "test-game-" + str(uuid.uuid4())

    item = {
        "sub": sub,
        "game_id": game_id,
        "result": "win",
        "score": 999,
    }

    # --- PUT ITEM ---
    table.put_item(Item=item)

    # --- GET ITEM ---
    response = table.get_item(Key={"sub": sub, "game_id": game_id})
    assert "Item" in response, "Item should exist after put_item"
    assert response["Item"]["result"] == "win"
    assert response["Item"]["score"] == 999

    # --- DELETE ITEM ---
    delete_response = table.delete_item(
        Key={"sub": sub, "game_id": game_id},
        ReturnValues="ALL_OLD"
    )

    # Confirm deletion (ALL_OLD contains the deleted item)
    assert "Attributes" in delete_response
    assert delete_response["Attributes"]["sub"] == sub
    assert delete_response["Attributes"]["game_id"] == game_id

    # --- CONFIRM IT'S REALLY GONE ---
    post_delete = table.get_item(Key={"sub": sub, "game_id": game_id})
    assert "Item" not in post_delete, "Item should be deleted"
